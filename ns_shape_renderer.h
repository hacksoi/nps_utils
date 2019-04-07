#ifndef NS_SHAPE_RENDERER_H
#define NS_SHAPE_RENDERER_H

#include "ns_common_renderer.h"
#include "ns_opengl_functions.h"
#include "ns_common.h"

#define SHAPE_RENDERER_MAX_VERTICES 4*1024*1024
#define SHAPE_RENDERER_FLOATS_PER_VERTEX 6
#define SHAPE_RENDERER_BYTES_PER_VERTEX (sizeof(float)*SHAPE_RENDERER_FLOATS_PER_VERTEX)
#define SHAPE_RENDERER_CAPACITY_BYTES (SHAPE_RENDERER_MAX_VERTICES*SHAPE_RENDERER_BYTES_PER_VERTEX)

enum shape_type
{
    ShapeType_None,
    ShapeType_Point,
    ShapeType_Line,
    ShapeType_Triangle,
    ShapeType_Triangle_Fan,
    ShapeType_NumTypes,
};

struct shape_renderer_group
{
    shape_type Type;
    uint32_t NumElements;
};

struct shape_renderer
{
    common_renderer Common;

    shape_type CurrentType;
    float *InsertionPoint;
    uint32_t NumVertices;
    float PointSize;
    float LineWidth;

    bool IsWireframe;
};

internal void
Reset(shape_renderer *ShapeRenderer)
{
    ShapeRenderer->InsertionPoint = (float *)ShapeRenderer->Common.VertexData;
    ShapeRenderer->NumVertices = 0;
}

internal shape_renderer
CreateShapeRenderer(int WindowWidth, int WindowHeight)
{
    shape_renderer Result;

    const char *VertexShaderSource = R"STR(
        #version 330 core
        layout(location = 0) in vec2 Pos;
        layout(location = 1) in vec4 vsColor;

        uniform float Zoom;
        uniform vec2 WindowDimensions;
        uniform vec2 CameraPos;

        out vec4 fsColor;

        void main()
        {
            vec2 ViewSpacePos = Pos - CameraPos;
            ViewSpacePos *= Zoom;
            ViewSpacePos += 0.5f*WindowDimensions;
            vec2 ClipPos = ((2.0f * ViewSpacePos) / WindowDimensions) - 1.0f;
            gl_Position = vec4(ClipPos, 0.0, 1.0f);

            fsColor = vsColor;
        }
        )STR";

    const char *FragmentShaderSource = R"STR(
        #version 330 core
        in vec4 fsColor;

        uniform int fsDebugValue;

        out vec4 OutputColor;

        void main()
        {
            OutputColor = fsColor;
        }
        )STR";

    Result.Common = CreateCommonRenderer(WindowWidth, WindowHeight, SHAPE_RENDERER_CAPACITY_BYTES, VertexShaderSource, FragmentShaderSource);
    Result.Common.Vbo = CreateAndBindVertexBuffer();
    Result.Common.Vao = CreateAndBindVertexArray();
    SetVertexAttributeFloat(0, 2, 6, 0);
    SetVertexAttributeFloat(1, 4, 6, 2);
    glBindVertexArray(0);

    Result.CurrentType = ShapeType_None;
    Result.IsWireframe = false;
    Reset(&Result);

    return Result;
}

internal uint32_t
GetBytesUsed(shape_renderer *ShapeRenderer)
{
    uint64_t Result = (uint64_t)((uint8_t *)ShapeRenderer->InsertionPoint - (uint8_t *)ShapeRenderer->Common.VertexData);
    Assert(Result == (uint32_t)Result);
    return (uint32_t)Result;
}

internal void
Flush(shape_renderer *ShapeRenderer)
{
    int ExpectedBytesUsed = 6*4*ShapeRenderer->NumVertices;
    int ActualBytesUsed = GetBytesUsed(ShapeRenderer);
    Assert(ActualBytesUsed == ExpectedBytesUsed);

#if 0
    /* For some reason, these glGet calls can take a lot of time https://gamedev.stackexchange.com/questions/34944/glgetfloatv-should-it-be-avoided-can-it-be-replaced. */
    /* 0 = Front, 1 = Back. */
    GLint OriginalPolygonMode[2];
    glGetIntegerv(GL_POLYGON_MODE, OriginalPolygonMode);
    Assert(OriginalPolygonMode[0] == OriginalPolygonMode[1]);
#endif
    if (ShapeRenderer->IsWireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    BeginRender(&ShapeRenderer->Common, GetBytesUsed(ShapeRenderer));

    switch (ShapeRenderer->CurrentType)
    {
        case ShapeType_Point:
        {
            glPointSize(ShapeRenderer->PointSize);
            glDrawArrays(GL_POINTS, 0, ShapeRenderer->NumVertices);
        } break;

        case ShapeType_Line:
        {
            glLineWidth(ShapeRenderer->LineWidth);
            glDrawArrays(GL_LINES, 0, ShapeRenderer->NumVertices);
        } break;

        case ShapeType_Triangle:
        {
            glDrawArrays(GL_TRIANGLES, 0, ShapeRenderer->NumVertices);
        } break;

        case ShapeType_Triangle_Fan:
        {
            glDrawArrays(GL_TRIANGLE_FAN, 0, ShapeRenderer->NumVertices);
        } break;
    }

    Reset(ShapeRenderer);
    EndRender(&ShapeRenderer->Common);
}

internal void
Render(shape_renderer *ShapeRenderer)
{
    Flush(ShapeRenderer);

    /* We're at the end of the frame. Reset. */
    ShapeRenderer->CurrentType = ShapeType_None;
}

internal bool
HasBytesLeftFor(shape_renderer *ShapeRenderer, uint32_t Size)
{
    uint32_t BytesUsed = GetBytesUsed(ShapeRenderer);
    bool Result = (BytesUsed + Size) <= SHAPE_RENDERER_CAPACITY_BYTES;
    return Result;
}

internal void
CheckFlush(shape_renderer *ShapeRenderer, shape_type ShapeType, int ShapeSizeBytes, bool ForceFlushIfSame)
{
    if (ShapeRenderer->CurrentType == ShapeType)
    {
        if (!HasBytesLeftFor(ShapeRenderer, ShapeSizeBytes) ||
            ForceFlushIfSame)
        {
            Flush(ShapeRenderer);
        }
    }
    else
    {
        if (ShapeRenderer->CurrentType != ShapeType_None)
        {
            Flush(ShapeRenderer);
        }
        ShapeRenderer->CurrentType = ShapeType;
    }
}

internal void
Add(shape_renderer *ShapeRenderer, v2 P, v4 Color)
{
    *ShapeRenderer->InsertionPoint++ = P.X;
    *ShapeRenderer->InsertionPoint++ = P.Y;
    *ShapeRenderer->InsertionPoint++ = Color.R;
    *ShapeRenderer->InsertionPoint++ = Color.G;
    *ShapeRenderer->InsertionPoint++ = Color.B;
    *ShapeRenderer->InsertionPoint++ = Color.A;
    ShapeRenderer->NumVertices += 1;
}

internal void
AddPoint(shape_renderer *ShapeRenderer, v2 P, v4 Color = GLUTILS_WHITE, float PointSize = 5.0f)
{
    bool ForceFlushIfSame = ShapeRenderer->PointSize != PointSize;
    CheckFlush(ShapeRenderer, ShapeType_Point, 1*SHAPE_RENDERER_BYTES_PER_VERTEX, ForceFlushIfSame);
    ShapeRenderer->PointSize = PointSize;
    Add(ShapeRenderer, P, Color);
}

internal void
AddLine(shape_renderer *ShapeRenderer, line2 Line, float LineWidth = 1.0f, v4 Color = GLUTILS_WHITE)
{
    bool ForceFlushIfSame = ShapeRenderer->LineWidth != LineWidth;
    CheckFlush(ShapeRenderer, ShapeType_Line, 2*SHAPE_RENDERER_BYTES_PER_VERTEX, ForceFlushIfSame);
    ShapeRenderer->LineWidth = LineWidth;
    Add(ShapeRenderer, Line.P1, Color);
    Add(ShapeRenderer, Line.P2, Color);
}

internal void
AddTriangle(shape_renderer *ShapeRenderer, tri2 Tri, v4 Color = GLUTILS_WHITE, bool IsWireframe = false)
{
    bool ForceFlushIfSame = ShapeRenderer->IsWireframe != IsWireframe;
    CheckFlush(ShapeRenderer, ShapeType_Triangle, 3*SHAPE_RENDERER_BYTES_PER_VERTEX, ForceFlushIfSame);
    ShapeRenderer->IsWireframe = IsWireframe;
    Add(ShapeRenderer, Tri.P1, Color);
    Add(ShapeRenderer, Tri.P2, Color);
    Add(ShapeRenderer, Tri.P3, Color);
}

internal void
AddQuad(shape_renderer *ShapeRenderer, quad2 Quad, v4 Color = GLUTILS_WHITE, bool IsOutline = false)
{
    if (IsOutline)
    {
        AddLine(ShapeRenderer, LINE2(Quad.BottomLeft, Quad.BottomRight), 1.0f, Color);
        AddLine(ShapeRenderer, LINE2(Quad.BottomRight, Quad.TopRight), 1.0f, Color);
        AddLine(ShapeRenderer, LINE2(Quad.TopRight, Quad.TopLeft), 1.0f, Color);
        AddLine(ShapeRenderer, LINE2(Quad.TopLeft, Quad.BottomLeft), 1.0f, Color);
    }
    else
    {
        AddTriangle(ShapeRenderer, TRI2(Quad.BottomLeft, Quad.BottomRight, Quad.TopRight), Color);
        AddTriangle(ShapeRenderer, TRI2(Quad.BottomLeft, Quad.TopRight, Quad.TopLeft), Color);
    }
}

internal void
AddRect(shape_renderer *ShapeRenderer, rect2 Rect, v4 Color = GLUTILS_WHITE, bool IsOutline = false)
{
    quad2 Quad = QUAD2(Rect);
    AddQuad(ShapeRenderer, Quad, Color, IsOutline);
}

internal void
AddRay(shape_renderer *ShapeRenderer, ray2 Ray, float Length, float LineWidth, v4 Color = GLUTILS_WHITE)
{
    v2 P1 = Ray.Pos;
    v2 P2 = Ray.Pos + Length*Ray.Dir;
    line2 Line = { P1, P2 };
    AddLine(ShapeRenderer, Line, LineWidth, Color);
}

internal void
AddCircle(shape_renderer *ShapeRenderer, v2 Pos, float Radius, v4 Color = GLUTILS_WHITE, bool IsWireframe = false)
{
    int NumCirclePoints = 64;
    int MaxNumVertices = NumCirclePoints + 2;
    bool ForceFlushIfSame = ShapeRenderer->IsWireframe != IsWireframe;
    CheckFlush(ShapeRenderer, ShapeType_Triangle_Fan, MaxNumVertices*SHAPE_RENDERER_BYTES_PER_VERTEX, ForceFlushIfSame);
    ShapeRenderer->IsWireframe = IsWireframe;

    float Step = 360.0f/(float)NumCirclePoints;
    v2 PointOnCircle = V2(0.0f, Radius);
    line2 LineThroughCircle = {Pos - PointOnCircle, Pos + PointOnCircle};
    Add(ShapeRenderer, Pos, Color);
    NumCirclePoints++;
    while (NumCirclePoints--)
    {
        Add(ShapeRenderer, LineThroughCircle.P1, Color);
        RotateAroundCenter(&LineThroughCircle, Step);
    }
}

internal void
Free(shape_renderer *ShapeRenderer)
{
    Free(&ShapeRenderer->Common);
}

void SetCameraPos(shape_renderer *ShapeRenderer, v2 NewCameraPos)
{
    ShapeRenderer->Common.CameraPos = NewCameraPos;
}

void SetZoom(shape_renderer *ShapeRenderer, float NewZoom)
{
    ShapeRenderer->Common.Zoom = NewZoom;
}

void SetWindowSize(shape_renderer *ShapeRenderer, int NewWindowWidth, int NewWindowHeight)
{
    SetWindowSize(&ShapeRenderer->Common, NewWindowWidth, NewWindowHeight);
}

v2 ConvertScreenToGame(shape_renderer *ShapeRenderer, v2 ScreenPoint)
{
    v2 ScreenCenter = V2((float)ShapeRenderer->Common.WindowWidth/2.0f, (float)ShapeRenderer->Common.WindowHeight/2.0f);
    ScreenPoint -= ScreenCenter;
    float InverseZoom = 1.0f/ShapeRenderer->Common.Zoom;
    ScreenPoint *= InverseZoom;
    ScreenPoint += ShapeRenderer->Common.CameraPos;
    return ScreenPoint;
}

#endif