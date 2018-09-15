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

#define NS_FONT_RENDERER_MAX_STRING_SIZE 256
#define NS_FONT_RENDERER_FLOATS_PER_CHAR 24
#define NS_FONT_RENDERER_MAX_VERTEX_DATA_SIZE (NS_FONT_RENDERER_MAX_STRING_SIZE*NS_FONT_RENDERER_FLOATS_PER_CHAR)

enum font_renderer_type
{
    FontRendererType_BMF,
    FontRendererType_TTF,
};

struct font_renderer
{
    common_renderer Common;

    ns_bmf BmfFile;
    GLuint BmfTextureId;

    ns_ttf TtfFile;

    uint32_t ImageWidth;
    uint32_t ImageHeight;

    font_renderer_type Type;
};

const char *NsFontRendererVertexShaderSource = R"STR(
#version 330 core
layout (location = 0) in vec2 Pos;
layout (location = 1) in vec2 vsTexCoord;

uniform vec2 WindowDimensions;

out vec2 fsTexCoord;

void main()
{
    vec2 ClipPos = ((2.0f*Pos)/WindowDimensions) - 1.0f;
    gl_Position = vec4(ClipPos, 0.0, 1.0f);

    fsTexCoord = vsTexCoord;
}
)STR";

const char *NsFontRendererFragmentShaderSource = R"STR(
#version 330 core
in vec2 fsTexCoord;

uniform sampler2D Texture;

out vec4 OutputColor;

void main()
{
    vec4 TexColor = texture(Texture, fsTexCoord);
    OutputColor = vec4(1.0f, 1.0f, 1.0f, TexColor.r);
}
)STR";

internal font_renderer
CreateFontRenderer(uint32_t WindowWidth, uint32_t WindowHeight)
{
    font_renderer Result;
    Result.Common = CreateCommonRenderer(WindowWidth, WindowHeight, sizeof(float)*NS_FONT_RENDERER_MAX_VERTEX_DATA_SIZE, NsFontRendererVertexShaderSource, NsFontRendererFragmentShaderSource);
    Result.Common.Vbo = CreateAndBindVertexBuffer();
    Result.Common.Vao = CreateAndBindVertexArray();
    SetVertexAttributeFloat(0, 2, 4, 0);
    SetVertexAttributeFloat(1, 2, 4, 2);
    glBindVertexArray(0);
    return Result;
}

internal font_renderer
CreateFontRenderer(const char *TtfFilename, uint32_t WindowWidth, uint32_t WindowHeight)
{
    font_renderer Result = CreateFontRenderer(WindowWidth, WindowHeight);
    Result.TtfFile = LoadTtf(TtfFilename);
    Result.Type = FontRendererType_TTF;
    return Result;
}

internal font_renderer
CreateFontRenderer(const char *BmfFilename, const char *TgaFilename, uint32_t WindowWidth, uint32_t WindowHeight)
{
    font_renderer Result = CreateFontRenderer(WindowWidth, WindowHeight);

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

struct glyph_outline
{
    v2 *Points;
    int MaxNumPoints;
    int NumPoints;
    int *ContourEndIndices;
    int NumContours;
    rect2 BoundingBox;
};

#define NUM_BEZIER_STEPS 5
#define BEZIER_STEP_AMT (1.0f/(float)NUM_BEZIER_STEPS)

internal int
CalcMaxNumBezierPoints(int NumGlyphPoints)
{
    /* + 1 for the very first point. */
    int Result = NumGlyphPoints*NUM_BEZIER_STEPS + 1;
    return Result;
}

/* TODO: query OS for this. */
#define SCREEN_DPI 72

/* TODO: Instead of specifying point size, how about pixels? */
internal glyph_outline
CreateGlyphOutline(ns_ttf *TtfFile, char Char, int PointSize)
{
    int DebugNumPoints = GetNumPointsInGlyph(TtfFile, Char);
    DebugNumPoints = CalcMaxNumBezierPoints(DebugNumPoints);
    int DebugNumContours = GetNumContoursInGlyph(TtfFile, Char);

    /* TODO(@memory): We're calculating the maximum number of points, but for two points connected by a simple line, we 
       don't split the line up into 10 pieces. */
    glyph_outline Result = {};
    int NumGlyphPoints = GetNumPointsInGlyph(TtfFile, Char);
    int NumContours = GetNumContoursInGlyph(TtfFile, Char);
    Result.MaxNumPoints = CalcMaxNumBezierPoints(NumGlyphPoints);
    int PointsAndEndContourIndicesSize = sizeof(v2)*Result.MaxNumPoints + sizeof(int)*NumContours;
    Result.Points = (v2 *)MemAlloc(PointsAndEndContourIndicesSize);
    Result.ContourEndIndices = (int *)(Result.Points + Result.MaxNumPoints);
    float FUnitsToPixels = GetFUnitsToPixels(TtfFile, PointSize, SCREEN_DPI);
    Result.BoundingBox = FUnitsToPixels*GetBoundingBox(TtfFile, Char);

	for (glyph_iterator GlyphIterator = GetGlyphIterator(TtfFile, Char); HasMore(&GlyphIterator); Advance(&GlyphIterator))
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

internal void
Free(glyph_outline *GlyphMesh)
{
    MemFree(GlyphMesh->Points);
}

struct glyph_edge_iterator
{
    glyph_outline *Mesh;
    int PointIdx1;
    int PointIdx2;
};

internal glyph_edge_iterator
GetEdgeIterator(glyph_outline *Mesh)
{
    glyph_edge_iterator Result = {};
    Result.Mesh = Mesh;
    Result.PointIdx2 = 1;
    return Result;
}

internal bool
HasMore(glyph_edge_iterator *Iterator)
{
    Assert(Iterator->PointIdx2 <= Iterator->Mesh->ContourEndIndices[Iterator->Mesh->NumContours - 1]);
    bool Result = (Iterator->PointIdx2 != Iterator->Mesh->ContourEndIndices[Iterator->Mesh->NumContours - 1]);
    return Result;
}

internal void
Advance(glyph_edge_iterator *Iterator)
{
    Assert(Iterator->PointIdx2 < Iterator->Mesh->NumPoints - 1);
    bool AtEndOfContour = false;
    for (int I = 0; I < Iterator->Mesh->NumContours; I++)
    {
        if (Iterator->PointIdx2 == Iterator->Mesh->ContourEndIndices[I])
        {
            AtEndOfContour = true;
            break;
        }
    }
    Iterator->PointIdx1 += AtEndOfContour ? 2 : 1;
    Iterator->PointIdx2 = Iterator->PointIdx1 + 1;
}

internal line2
GetEdge(glyph_edge_iterator *Iterator)
{
    v2 P1 = Iterator->Mesh->Points[Iterator->PointIdx1];
    v2 P2 = Iterator->Mesh->Points[Iterator->PointIdx2];
    line2 Result = LINE2(P1, P2);
    return Result;
}

internal ns_texture
CreateGlyphTexture(glyph_outline *Mesh)
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
            for (glyph_edge_iterator EdgeIterator = GetEdgeIterator(Mesh); HasMore(&EdgeIterator); Advance(&EdgeIterator))
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

struct shape_mesh
{
    int NumTriangles;
    tri2 *Triangles;
};

internal shape_mesh
CreateMesh(ns_ttf *Ttf, glyph_outline *Outline)
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

internal void
DrawString(font_renderer *FontRenderer, const char *String, float StringPosX, float StringPosY, float FontScale = 1.0f, shape_renderer *ShapeRenderer = 0)
{
    Assert(StrLen((char *)String) <= NS_FONT_RENDERER_MAX_STRING_SIZE);

    v2 StringPos = V2(500.0f, 250.0f);//V2(StringPosX, StringPosY);
    switch (FontRenderer->Type)
    {
        case FontRendererType_TTF:
        {
            Assert(ShapeRenderer);
            while (*String)
            {
                char Char = *String++;

				glyph_outline GlyphOutline = CreateGlyphOutline(&FontRenderer->TtfFile, Char, 300);
                shape_mesh GlyphMesh = CreateMesh(&FontRenderer->TtfFile, &GlyphOutline);
#if 0
                ns_texture GlyphTexture = CreateGlyphTexture(&GlyphOutline);
                ns_texture GlyphTexture;
                GlyphTexture.Width = 64;
                GlyphTexture.Height = 32;
                GlyphTexture.Data = MemAlloc(GlyphTexture.Width*GlyphTexture.Height);

                // debug
                for (int Y = 0; Y < GlyphTexture.Height; Y++)
                {
                    for (int X = 0; X < GlyphTexture.Width; X++)
                    {
                        GlyphTexture.Data[Y*GlyphTexture.Width + X] = (Y % 2 == 0) ? 255 : 0;
                    }
                }

                GLuint TextureId;
                glGenTextures(1, &TextureId);
                glBindTexture(GL_TEXTURE_2D, TextureId);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, GlyphTexture.Width, GlyphTexture.Height, 0, GL_RED, GL_UNSIGNED_BYTE, GlyphTexture.Data);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glGenerateMipmap(GL_TEXTURE_2D);

                rect2 TexCoordRect = RECT2(V2(0.0f, 0.0f), V2(1.0f, 1.0f));

                rect2 PosRect = RectFromPosSize(StringPos, V2(GlyphTexture.Width, GlyphTexture.Height));
                int NumVertexData = 0;
                InsertTexture((float *)FontRenderer->Common.VertexData, &NumVertexData, PosRect, TexCoordRect);

                BeginRender(&FontRenderer->Common, sizeof(float)*NumVertexData);
                glBindTexture(GL_TEXTURE_2D, TextureId);
                glDrawArrays(GL_TRIANGLES, 0, NumVertexData/4);
                EndRender(&FontRenderer->Common);

                glDeleteTextures(1, &TextureId);
                Free(&GlyphTexture);
#endif
                Free(&GlyphOutline);
            }
        } break;

        case FontRendererType_BMF:
        {
            int NumVertexData = 0;
            while (*String)
            {
                uint32_t C = *String++;

                bmf_char *FirstChar = FontRenderer->BmfFile.GetChar(0);
                Assert(C >= FirstChar->id);
                bmf_char *BmfChar = FontRenderer->BmfFile.GetChar(C - FirstChar->id);

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

                v2 PosSize = FontScale*V2(BmfChar->width, BmfChar->height);
                quad2 PosQuad = QuadFromPosSize(V2(StringPosX, StringPosY), PosSize);
                InsertTexture((float *)FontRenderer->Common.VertexData, &NumVertexData, PosQuad, TexCoordQuad);

                StringPosX = PosQuad.BottomRight.X;
            }

            BeginRender(&FontRenderer->Common, sizeof(float)*NumVertexData);
            glBindTexture(GL_TEXTURE_2D, FontRenderer->BmfTextureId);
            glDrawArrays(GL_TRIANGLES, 0, NumVertexData/4);
            EndRender(&FontRenderer->Common);
        } break;

        default:
        {
            CrashProgram();
        } break;
    }
}

internal void
Free(font_renderer *FontRenderer)
{
    Free(&FontRenderer->TtfFile);
}

#endif