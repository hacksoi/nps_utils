/* For the TTF renderer, bitmaps just won't cut it. This is because we want to stretch the font out a lot, so if we
   stretch is diagonally for example, the bitmap is gonna have a lot of empty space that we have iterate over, and
   for each pixel transition we have to go through each edge; although, we can optimize that, but it'll still be
   expensive. Therefore, we we're doing a proper mesh using delaunay triangulation. */

#ifndef NS_FONT_RENDERER
#define NS_FONT_RENDERER

#include "ns_file.h"
#include "ns_tga.h"
#include "ns_bmf.h"
#include "ns_ttf.h"
#include "ns_texture.h"
#include "ns_opengl.h"
#include "ns_triangulation.h"

#define NS_FONT_RENDERER_MAX_STRING_SIZE 256
#define NS_FONT_RENDERER_FLOATS_PER_CHAR 24
#define NS_FONT_RENDERER_MAX_VERTEX_DATA_SIZE (NS_FONT_RENDERER_MAX_STRING_SIZE*NS_FONT_RENDERER_FLOATS_PER_CHAR)

/* TODO: query OS for this. */
#define SCREEN_DPI 72

#define NUM_BEZIER_STEPS 5
#define BEZIER_STEP_AMT (1.0f/(float)NUM_BEZIER_STEPS)

enum font_renderer_type
{
    FontRendererType_BMF,
    FontRendererType_TTF,
};

struct font_renderer
{
    common_renderer Common;

    union
    {
        struct
        {
            ns_bmf BmfFile;
            GLuint BmfTextureId;
        };

        ns_ttf TtfFile;
    };

    uint32_t ImageWidth;
    uint32_t ImageHeight;

    font_renderer_type Type;
};

struct glyph_outline
{
    /* Note: the first point of a contour is not included twice */
    v2 *Points;
    int NumPoints;

    line2 *Edges;
    int NumEdges;
};

/* A glyph outline specified by points. */
struct glyph_outline_points
{
    /* Note: the first point of a contour is included twice: once at the beginning and the end - this is necessary because
       the last and first points might have a curve between them. */
    v2 *Points;
    int NumPoints;

    rect2 BoundingBox;
    int *ContourEndIndices;
    int NumContours;
};

struct glyph_outline_points_edge_iterator
{
    glyph_outline_points *OutlinePoints;
    int PointIdx1;
    int PointIdx2;
};

/* TODO(@memory): this uses more memory than glyph_outline_points - we use it because it's more convenient. A glyph 
   outline specified by edges. */
struct glyph_outline_edges
{
    line2 *Edges;
    int NumEdges;
};

internal int
CalcMaxNumBezierPoints(int NumGlyphPoints)
{
    /* + 1 for the very first point. */
    int Result = NumGlyphPoints*NUM_BEZIER_STEPS + 1;
    return Result;
}

internal glyph_outline_points_edge_iterator
GetEdgeIterator(glyph_outline_points *Mesh)
{
    glyph_outline_points_edge_iterator Result = {};
    Result.OutlinePoints = Mesh;
    Result.PointIdx2 = 1;
    return Result;
}

internal bool
HasMore(glyph_outline_points_edge_iterator *Iterator)
{
    Assert(Iterator->PointIdx2 <= Iterator->OutlinePoints->ContourEndIndices[Iterator->OutlinePoints->NumContours - 1]);
    /* Did we wrap - last and first points? */
    bool Result = (Iterator->PointIdx2 != 0);
    return Result;
}

internal void
Advance(glyph_outline_points_edge_iterator *Iterator)
{
    Assert(Iterator->PointIdx2 < Iterator->OutlinePoints->NumPoints);
    /* Are we at the very end? */
    if (Iterator->PointIdx2 == Iterator->OutlinePoints->NumPoints - 1)
    {
        /* Well, there's one more edge - the last and first points. */
        Iterator->PointIdx1 = Iterator->PointIdx2;
        Iterator->PointIdx2 = 0;
    }
    else
    {
        bool AtEndOfContour = false;
        for (int I = 0; I < Iterator->OutlinePoints->NumContours; I++)
        {
            if (Iterator->PointIdx2 == Iterator->OutlinePoints->ContourEndIndices[I])
            {
                AtEndOfContour = true;
                break;
            }
        }
        Iterator->PointIdx1 += AtEndOfContour ? 2 : 1;
        Iterator->PointIdx2 = Iterator->PointIdx1 + 1;
    }
}

internal line2
GetEdge(glyph_outline_points_edge_iterator *Iterator)
{
    v2 P1 = Iterator->OutlinePoints->Points[Iterator->PointIdx1];
    v2 P2 = Iterator->OutlinePoints->Points[Iterator->PointIdx2];
    line2 Result = LINE2(P1, P2);
    return Result;
}

#if 0 /* Doesn't work. */
internal ns_texture
CreateGlyphTexture(glyph_outline_points *Mesh)
{
    /* Make it zero based. */
    v2 Fix = V2(-Mesh->BoundingBox.Min.X, -Mesh->BoundingBox.Min.Y);
    Mesh->BoundingBox.Min += V2(0, 0);
    Mesh->BoundingBox.Max += Fix;
    for (int I = 0; I < Mesh->NumPoints; I++)
    {
        Mesh->Points[I] += Fix;
    }

    v2 Dimensions = GetSize(Mesh->BoundingBox);
    ns_texture Result = {};
    Result.Width = Ceil(Dimensions.X);
    Result.Height = Ceil(Dimensions.Y);
    Result.Width = NextPowerOfTwo(Result.Width);
    Result.Height = NextPowerOfTwo(Result.Height);
    Result.Data = MemAlloc(Result.Width*Result.Height);
    for (int Y = 0; Y < Result.Height; Y++)
    {
        v2 PrevPixelLoc = V2(-1, Y);
        bool InGlyph = false;
        for (int X = 0; X < Result.Width; X++)
        {
            /* TODO: somehow binary search for each horizontal? */
            v2 PixelLoc = V2(X, Y);
            line2 L = LINE2(PrevPixelLoc, PixelLoc);
            int NumIntersections = 0;
            for (glyph_outline_points_edge_iterator EdgeIterator = GetEdgeIterator(Mesh); HasMore(&EdgeIterator); Advance(&EdgeIterator))
            {
                line2 Edge = GetEdge(&EdgeIterator);
                if (L.P1.X == Edge.P1.X || L.P1.Y == Edge.P1.Y ||
                    L.P1.X == Edge.P2.X || L.P1.Y == Edge.P2.Y ||
                    Intersects(L, Edge))
                {
                    NumIntersections++;
                }
            }

            if (NumIntersections % 2)
            {
                /* Toogle. */
                InGlyph = !InGlyph;
            }

            uint8_t *Pixel = &Result.Data[Y*Result.Width + X];
            *Pixel = InGlyph ? 255 : 0;

            PrevPixelLoc = PixelLoc;
        }
    }

    return Result;
}
#endif

struct shape_mesh
{
    int NumTriangles;
    tri2 *Triangles;
};

internal shape_mesh
CreateMesh(ns_ttf *Ttf, glyph_outline_points *Outline)
{
    shape_mesh Result = {};
    Result.Triangles = (tri2 *)MemAlloc(Megabytes(1));
    return Result;
}

internal void
Free(shape_mesh *Mesh)
{
    MemFree(Mesh->Triangles);
}

/* APIs */

internal font_renderer
CreateFontRenderer(const char *TtfFilename, uint32_t WindowWidth, uint32_t WindowHeight)
{
    font_renderer Result;
    Result.Common = CreateGenericTextureCommonRenderObjects(WindowWidth, WindowHeight, NS_FONT_RENDERER_MAX_VERTEX_DATA_SIZE);
    Result.TtfFile = LoadTtf(TtfFilename);
    Result.Type = FontRendererType_TTF;
    return Result;
}

internal font_renderer
CreateFontRenderer(const char *BmfFilename, const char *TgaFilename, uint32_t WindowWidth, uint32_t WindowHeight)
{
    font_renderer Result;
    Result.Common = CreateGenericTextureCommonRenderObjects(WindowWidth, WindowHeight, NS_FONT_RENDERER_MAX_VERTEX_DATA_SIZE);

    ns_tga TgaFile = LoadTga(TgaFilename);
    Result.ImageWidth = TgaFile.Header->Width;
    Result.ImageHeight = TgaFile.Header->Height;
    glGenTextures(1, &Result.BmfTextureId);
    glBindTexture(GL_TEXTURE_2D, Result.BmfTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, TgaFile.Header->Width, TgaFile.Header->Height, 0, GL_RED, GL_UNSIGNED_BYTE, TgaFile.Data);
    glGenerateMipmap(GL_TEXTURE_2D);
    Free(TgaFile);

    Result.BmfFile = LoadBmf(BmfFilename);
    Result.Type = FontRendererType_BMF;

    return Result;
}

/* TODO: Instead of specifying point size, how about pixels? */
glyph_outline_points CreateGlyphOutlinePoints(font_renderer *FontRenderer, char Char, int PointSize)
{
    int DebugNumPoints = GetNumPointsInGlyph(&FontRenderer->TtfFile, Char);
    DebugNumPoints = CalcMaxNumBezierPoints(DebugNumPoints);
    int DebugNumContours = GetNumContoursInGlyph(&FontRenderer->TtfFile, Char);

    /* TODO(@memory): We're calculating the maximum number of points, but for two points connected by a simple line, we 
       shouldn't split the line up into 10 pieces. */
    glyph_outline_points Result = {};
    int NumGlyphPoints = GetNumPointsInGlyph(&FontRenderer->TtfFile, Char);
    int NumContours = GetNumContoursInGlyph(&FontRenderer->TtfFile, Char);
    int MaxNumPoints = CalcMaxNumBezierPoints(NumGlyphPoints);
    int PointsAndEndContourIndicesSize = sizeof(v2)*MaxNumPoints + sizeof(int)*NumContours;
    Result.Points = (v2 *)MemAlloc(PointsAndEndContourIndicesSize);
    Result.ContourEndIndices = (int *)(Result.Points + MaxNumPoints);
    float FUnitsToPixels = GetFUnitsToPixels(&FontRenderer->TtfFile, PointSize, SCREEN_DPI);
    Result.BoundingBox = FUnitsToPixels*GetBoundingBox(&FontRenderer->TtfFile, Char);

	for (glyph_iterator GlyphIterator = GetGlyphIterator(&FontRenderer->TtfFile, Char); HasMore(&GlyphIterator); Advance(&GlyphIterator))
	{
		v2 GlyphOffset = FUnitsToPixels*V2(GlyphIterator.XOffset, GlyphIterator.YOffset);
		for (glyph_contour_iterator ContourIterator = GetContourIterator(&GlyphIterator); HasMore(&ContourIterator); Advance(&ContourIterator))
		{
			glyph_contour_point_iterator *PointIterator = GetPointIterator(&ContourIterator);
			v2 PrevPoints[4] = {};
			int NumOffCurvePoints = 0;
			v2 FirstPoint = FUnitsToPixels*V2(GetX(PointIterator), GetY(PointIterator)) + GlyphOffset;
			PrevPoints[ArrayCount(PrevPoints) - 1] = FirstPoint;
            Result.Points[Result.NumPoints++] = FirstPoint;
			Advance(PointIterator);
			while (1)
			{
				v2 Point;
				uint8_t Flag;
				if (!HasMore(PointIterator))
				{
					Point = FirstPoint;
					Flag = ON_CURVE_POINT;
				}
				else
				{
					Point = FUnitsToPixels*V2(GetX(PointIterator), GetY(PointIterator)) + GlyphOffset;
					Flag = GetFlag(PointIterator);
				}

				if (Flag & ON_CURVE_POINT)
				{
					Assert(NumOffCurvePoints <= ArrayCount(PrevPoints));
                    /* We don't have t = 0 because we go all the way up to t = 1. */
					float t = BEZIER_STEP_AMT;
					switch (NumOffCurvePoints)
					{
						case 0:
						{
							v2 A = PrevPoints[ArrayCount(PrevPoints) - 1];
							v2 B = Point;
                            Result.Points[Result.NumPoints++] = Point;
						} break;

						/* Quadratic. */
						case 1:
						{
							v2 A = PrevPoints[ArrayCount(PrevPoints) - 2];
							v2 B = PrevPoints[ArrayCount(PrevPoints) - 1];
							v2 C = Point;

							for (int I = 1; I <= NUM_BEZIER_STEPS; I++, t += BEZIER_STEP_AMT)
							{
								v2 Interpolated = Bezier(A, B, C, t);
								v4 Color = GLUTILS_WHITE;
                                Result.Points[Result.NumPoints++] = Interpolated;
							}
						} break;

						/* Cubic. */
						case 2:
						{
							v2 A = PrevPoints[ArrayCount(PrevPoints) - 3];
							v2 B = PrevPoints[ArrayCount(PrevPoints) - 2];
							v2 C = PrevPoints[ArrayCount(PrevPoints) - 1];
							v2 D = Point;

							for (int I = 1; I <= NUM_BEZIER_STEPS; I++, t += BEZIER_STEP_AMT)
							{
								v2 Interpolated = Bezier(A, B, C, D, t);
								v4 Color = GLUTILS_WHITE;
                                Result.Points[Result.NumPoints++] = Interpolated;
							}
						} break;

						/* Quartic. */
						case 3:
						{
							v2 A = PrevPoints[ArrayCount(PrevPoints) - 4];
							v2 B = PrevPoints[ArrayCount(PrevPoints) - 3];
							v2 C = PrevPoints[ArrayCount(PrevPoints) - 2];
							v2 D = PrevPoints[ArrayCount(PrevPoints) - 1];
							v2 E = Point;

							for (int I = 1; I <= NUM_BEZIER_STEPS; I++, t += BEZIER_STEP_AMT)
							{
								v2 Interpolated = Bezier(A, B, C, D, E, t);
								v4 Color = GLUTILS_WHITE;
                                Result.Points[Result.NumPoints++] = Interpolated;
							}
						} break;
					}
					NumOffCurvePoints = 0;
				}
				else
				{
					NumOffCurvePoints++;
				}
                Assert(Result.NumPoints <= DebugNumPoints);

				for (int I = 0; I < ArrayCount(PrevPoints) - 1; I++)
				{
					PrevPoints[I] = PrevPoints[I + 1];
				}
				PrevPoints[ArrayCount(PrevPoints) - 1] = Point;

				if (!HasMore(PointIterator))
				{
					break;
				}
				Advance(PointIterator);
			}
            Result.ContourEndIndices[Result.NumContours++] = Result.NumPoints - 1;
            Assert(Result.NumContours <= DebugNumContours);
		}
	}
    Assert(Result.NumContours == DebugNumContours);
    return Result;
}

glyph_outline_edges ConvertOutlinePointsToEdges(glyph_outline_points *GlyphOutlinePoints)
{
    glyph_outline_edges Result;

    line2 *Edges = (line2 *)MemAlloc(sizeof(line2)*GlyphOutlinePoints->NumPoints);
    int NumEdges = 0;
    for (glyph_outline_points_edge_iterator EdgeIterator = GetEdgeIterator(GlyphOutlinePoints); HasMore(&EdgeIterator); Advance(&EdgeIterator))
    {
        Edges[NumEdges++] = GetEdge(&EdgeIterator);
    }

    Result.Edges = Edges;
    Result.NumEdges = NumEdges;
    return Result;
}

glyph_outline CreateGlyphOutline(font_renderer *FontRenderer, char Char, int PointSize)
{
    glyph_outline Result = {};
    glyph_outline_points GlyphOutlinePoints = CreateGlyphOutlinePoints(FontRenderer, Char, PointSize);
    glyph_outline_edges GlyphOutlineEdges = ConvertOutlinePointsToEdges(&GlyphOutlinePoints);

    /* The outline for the contours have the first point duplicated in the first and last indices. User (me) does not want 
       this, so remove them. */
    {
        /* @debug: ... of course, we should make sure that's true! */
        {
            int CurContourStartIdx = 0;
            for (int ContourIdx = 0; ContourIdx < GlyphOutlinePoints.NumContours; ContourIdx++)
            {
                int ContourEndIdx = GlyphOutlinePoints.ContourEndIndices[ContourIdx];
                Assert(GlyphOutlinePoints.Points[ContourEndIdx] == GlyphOutlinePoints.Points[CurContourStartIdx]);
                CurContourStartIdx = ContourEndIdx + 1;
            }
        }

        for (int ContourIdx = 0; ContourIdx < GlyphOutlinePoints.NumContours; ContourIdx++)
        {
            int ContourEndIdx = GlyphOutlinePoints.ContourEndIndices[ContourIdx];

            /* Remove contour end point. I think we should retain ordering. don't want to surpise user (me). */
            for (int PointsIdx = ContourEndIdx; PointsIdx < GlyphOutlinePoints.NumPoints - 1; PointsIdx++)
            {
                GlyphOutlinePoints.Points[PointsIdx] = GlyphOutlinePoints.Points[PointsIdx + 1];
            }
            GlyphOutlinePoints.NumPoints--;

            /* We also have to decrement the end indices! */
            for (int DecrementContourIdx = ContourIdx + 1; DecrementContourIdx < GlyphOutlinePoints.NumContours; DecrementContourIdx++)
            {
                GlyphOutlinePoints.ContourEndIndices[DecrementContourIdx]--;
            }
        }
    }

    Assert(!CheckForDuplicates(GlyphOutlinePoints.Points, GlyphOutlinePoints.NumPoints));
    Assert(!CheckForDuplicates(GlyphOutlineEdges.Edges, GlyphOutlineEdges.NumEdges));

    Result.Points = GlyphOutlinePoints.Points;
    Result.NumPoints = GlyphOutlinePoints.NumPoints;
    Result.Edges = GlyphOutlineEdges.Edges;
    Result.NumEdges = GlyphOutlineEdges.NumEdges;
    return Result;
}

struct char_render_data
{
    v2 XYOffset;
    float XAdvance;

    quad2 TexCoordsQuad;
    v2 DefaultSize;
};

char_render_data GetCharRenderData(font_renderer *FontRenderer, char Char)
{
    /* Only BMF types have textures. */
    Assert(FontRenderer->Type == FontRendererType_BMF);

    bmf_char *FirstChar = FontRenderer->BmfFile.GetChar(0);
    Assert((uint32_t)Char >= FirstChar->id);
    bmf_char *BmfChar = FontRenderer->BmfFile.GetChar(Char - FirstChar->id);

    int FixedTexCoordCharY = BmfChar->y + BmfChar->height;
    FixedTexCoordCharY = FontRenderer->ImageHeight - FixedTexCoordCharY;

    float TexCoordCharX = (float)BmfChar->x/(float)FontRenderer->ImageWidth;
    float TexCoordCharY = (float)FixedTexCoordCharY/(float)FontRenderer->ImageHeight;
    float TexCoordCharWidth = (float)BmfChar->width/(float)FontRenderer->ImageWidth;
    float TexCoordCharHeight = (float)BmfChar->height/(float)FontRenderer->ImageHeight;

    quad2 TexCoordQuad = QuadFromPosSize(V2(TexCoordCharX, TexCoordCharY), V2(TexCoordCharWidth, TexCoordCharHeight));

    /* OpenGL expects beginning of image data to be the top of the image. It isn't. Do this to fix it. */
    TexCoordQuad.BottomLeft.Y *= -1.0f;
    TexCoordQuad.BottomRight.Y *= -1.0f;
    TexCoordQuad.TopRight.Y *= -1.0f;
    TexCoordQuad.TopLeft.Y *= -1.0f;

    v2 DefaultSize = V2(BmfChar->width, BmfChar->height);

    char_render_data Result;
    Result.TexCoordsQuad = TexCoordQuad;
    Result.DefaultSize = DefaultSize;

    /* This offset is interesting. It assumes that the top of the letter is touching the top-line (not the
       base-line). For convenience sake, we assume the bottom of the letter is touching the top-line. */
    Result.XYOffset = V2(BmfChar->xoffset, -(BmfChar->yoffset + BmfChar->height));

    Result.XAdvance = BmfChar->xadvance;
    return Result;
}

struct glyph_triangulation
{
    tri2 *Triangles;
    int NumTriangles;

    v2 *Points;
    int NumPoints;

    line2 *Edges;
    int NumEdges;
};
void Free(glyph_triangulation *GlyphTriangulation)
{
    free(GlyphTriangulation->Triangles);
    free(GlyphTriangulation->Points);
    free(GlyphTriangulation->Edges);
}
glyph_triangulation CreateGlyphTriangulation(font_renderer *FontRenderer, char Char, int PointSize, v2 GlyphPos, int DebugNumConstraintEdgesUser = 0)
{
    glyph_triangulation Result;

    glyph_outline GlyphOutline = CreateGlyphOutline(FontRenderer, Char, PointSize);
    for (int PointsIdx = 0; PointsIdx < GlyphOutline.NumPoints; PointsIdx++)
    {
        GlyphOutline.Points[PointsIdx] += GlyphPos;
    }
    for (int EdgesIdx = 0; EdgesIdx < GlyphOutline.NumEdges; EdgesIdx++)
    {
        GlyphOutline.Edges[EdgesIdx] += GlyphPos;
    }

    triangulation_result TriangulationResult = CreateTriangulation(GlyphOutline.Points, GlyphOutline.NumPoints,
                                                                   GlyphOutline.Edges, 
                                                                   GlyphOutline.NumEdges);

    /* And remove the triangles outside the outline. */
    for (int TriangulationTriIdx = 0; TriangulationTriIdx < TriangulationResult.NumTriangulationTris;)
    {
        tri2 TriangulationTri = TriangulationResult.TriangulationTris[TriangulationTriIdx];
        bool IsInside;
        {
            v2 TriCenter = GetCenter(TriangulationTri);
            int NumIntersections[2] = {};
            /* It's possible for it to intersect a vertex. Since each vertex is shared by at least two edges, it will intersect
               it twice, even though it should only be counted as once. Save these vertices so we can avoid. */
#define MAX_VERTICES_INTERSECTED 10
            v2 VerticesIntersected[MAX_VERTICES_INTERSECTED];
            int NumVerticesIntersected = 0;
            for (int DirectionIdx = 0; DirectionIdx < ArrayCount(NumIntersections); DirectionIdx++)
            {
                v2 Dir = DirectionIdx == 0 ? V2(1.0f, 0.0f) : V2(-1.0f, 0.0f);
                ray2 TriCenterRay = RAY2(TriCenter, Dir);
                for (int OutlineEdgesIdx = 0; OutlineEdgesIdx < GlyphOutline.NumEdges; OutlineEdgesIdx++)
                {
                    line2 OutlineEdge = GlyphOutline.Edges[OutlineEdgesIdx];
                    if (Intersects(TriCenterRay, OutlineEdge))
                    {
                        /* Did we intersect the vertex? */
                        bool AlreadyIntersected = false;
                        v2 IntersectedVert = {};
                        {
                            if (TriCenterRay.Pos.Y == OutlineEdge.P1.Y || TriCenterRay.Pos.Y == OutlineEdge.P2.Y)
                            {
                                /* Which one? */
                                IntersectedVert = (TriCenterRay.Pos.Y == OutlineEdge.P1.Y) ? OutlineEdge.P1 : OutlineEdge.P2;

                                /* Did we already intersect it? */
                                if (CheckArrayContains(VerticesIntersected, NumVerticesIntersected, IntersectedVert))
                                {
                                    AlreadyIntersected = true;
                                }
                            }
                        }
                        if (!AlreadyIntersected)
                        {
                            NumIntersections[DirectionIdx]++;

                            Assert(NumVerticesIntersected < ArrayCount(VerticesIntersected));
                            VerticesIntersected[NumVerticesIntersected++] = IntersectedVert;
                        }
                    }
                }
            }
            IsInside = ((NumIntersections[0] % 2) == 1) && ((NumIntersections[1] % 2) == 1);
        }
        if (!IsInside)
        {
            /* Remove this fucking bitch ass nigga. */
            Assert(TriangulationResult.NumTriangulationTris > 0);
            TriangulationResult.TriangulationTris[TriangulationTriIdx] = TriangulationResult.TriangulationTris[--TriangulationResult.NumTriangulationTris];
        }
        else
        {
            TriangulationTriIdx++;
        }
    }

    Result.Triangles = TriangulationResult.TriangulationTris;
    Result.NumTriangles = TriangulationResult.NumTriangulationTris;
    Result.Points = GlyphOutline.Points;
    Result.NumPoints = GlyphOutline.NumPoints;
    Result.Edges = GlyphOutline.Edges;
    Result.NumEdges = GlyphOutline.NumEdges;
    return Result;
}

float GetStringWidth(font_renderer *FontRenderer, char *String)
{
    Assert(FontRenderer->Type == FontRendererType_BMF);

    float Result = 0.0f;
    for (int StringIdx = 0; StringIdx < StrLen(String); StringIdx++)
    {
        char Char = String[StringIdx];
        char_render_data CharRenderData = GetCharRenderData(FontRenderer, Char);
        Result += CharRenderData.XAdvance;
    }
    return Result;
}

void DrawString(font_renderer *FontRenderer, char *String, v2 StringPos, float RotAngle = 0.0f, bool Centered = false, float StretchX = 1.0f)
{
    Assert(StrLen((char *)String) <= NS_FONT_RENDERER_MAX_STRING_SIZE);

    switch (FontRenderer->Type)
    {
        case FontRendererType_BMF:
        {
            int NumVertexData = 0;

            /* What we do is treat it like a regular mesh. We create the mesh centered on the origin, then scale, then rotate, then position. */

            /* Get total string width. */
            float TotalStringWidth = GetStringWidth(FontRenderer, String);

            /* Get the position of the string relative to the origin. */
            float LineHeight = FontRenderer->BmfFile.CommonBlock->lineHeight;
            const v2 OriginPos = V2(-(TotalStringWidth/2.0f), LineHeight/2.0f);
            const v2 OriginPosZeroY = V2(-(TotalStringWidth/2.0f), 0.0f);
            v2 CurPos = OriginPos;

            /* Generate the mesh at the origin. */
            quad2 PosSizeQuads[1024];
            quad2 TexCoordsQuads[1024];
            int NumQuads = 0;
            for (int StringIdx = 0; StringIdx < StrLen(String); StringIdx++)
            {
                char Char = String[StringIdx];

                char_render_data CharRenderData = GetCharRenderData(FontRenderer, Char);
                v2 Size = CharRenderData.DefaultSize;
                v2 Pos = CurPos + CharRenderData.XYOffset;
                quad2 PosSizeQuad = QuadFromPosSize(Pos, Size);

                PosSizeQuads[NumQuads] = PosSizeQuad;
                TexCoordsQuads[NumQuads] = CharRenderData.TexCoordsQuad;
                NumQuads++;

                CurPos.X += CharRenderData.XAdvance;
            }

            /* Do scaling. */
            for (int QuadIdx = 0; QuadIdx < NumQuads; QuadIdx++)
            {
                quad2 PosSizeQuad = PosSizeQuads[QuadIdx];
                PosSizeQuads[QuadIdx] = MultiplyXBy(PosSizeQuad, StretchX);
            }

            /* Do rotation. */
            for (int QuadIdx = 0; QuadIdx < NumQuads; QuadIdx++)
            {
                Rotate(&PosSizeQuads[QuadIdx], RotAngle);
            }
            v2 RotatedOriginPos = Rotate(OriginPos, RotAngle);
            v2 RotatedOriginPosZeroY = Rotate(OriginPosZeroY, RotAngle);

            /* Add user position. */
            for (int QuadIdx = 0; QuadIdx < NumQuads; QuadIdx++)
            {
                if (!Centered)
                {
                    PosSizeQuads[QuadIdx] += StretchX*(-RotatedOriginPosZeroY);
                }
                PosSizeQuads[QuadIdx] += StringPos;
            }

            /* Add quads. */
            for (int QuadIdx = 0; QuadIdx < NumQuads; QuadIdx++)
            {
                InsertTexture((float *)FontRenderer->Common.VertexData, &NumVertexData, PosSizeQuads[QuadIdx], TexCoordsQuads[QuadIdx]);
            }

            BeginRender(&FontRenderer->Common, NumVertexData);
            glBindTexture(GL_TEXTURE_2D, FontRenderer->BmfTextureId);
            Assert(NumVertexData % 4 == 0);
            glDrawArrays(GL_TRIANGLES, 0, NumVertexData/4);
            EndRender(&FontRenderer->Common);
        } break;

        default:
        {
            CrashProgram();
        } break;
    }
}

void DrawString(font_renderer *FontRenderer, char *String, v2 StringStartPos, v2 StringEndPos)
{
    switch (FontRenderer->Type)
    {
        case FontRendererType_BMF:
        {
            /* What we do is treat it like a regular mesh. We create the mesh centered on the origin, then scale, then rotate, then position. */

            /* Get scale factor. */
            float StringWidth = GetStringWidth(FontRenderer, String);
            float DesiredStringWidth = GetDistance(StringEndPos, StringStartPos);
            float StretchX = DesiredStringWidth/StringWidth;

            /* Get rotation angle. */
            v2 StartEndDiff = StringEndPos - StringStartPos;
            v2 StartEndDir = Normalize(StartEndDiff);
            float RotAngle = GetAngleBetween(V2(1.0f, 0.0f), StartEndDir, AngleDirection_CCW);

            DrawString(FontRenderer, String, StringStartPos, RotAngle, false, StretchX);
        } break;

        default:
        {
            CrashProgram();
        } break;
    }
}

void DrawString(font_renderer *FontRenderer, const char *String, v2 StringPosCenter, float RotAngle)
{
    DrawString(FontRenderer, (char *)String, StringPosCenter, RotAngle);
}

void DrawString(font_renderer *FontRenderer, const char *String, v2 StringStartPos, v2 StringEndPos)
{
    DrawString(FontRenderer, (char *)String, StringStartPos, StringEndPos);
}

uint32_t GetFontTextureId(font_renderer *FontRenderer)
{
    Assert(FontRenderer->Type == FontRendererType_BMF);
    uint32_t Result = FontRenderer->BmfTextureId;
    return Result;
}

internal void
Free(font_renderer *FontRenderer)
{
    Free(&FontRenderer->TtfFile);
}

#endif