#ifndef NS_SHAPE_RENDERER_H
#define NS_SHAPE_RENDERER_H

#include "common.h"
#include "opengl_functions.h"

#define SHAPE_RENDERER_CAPACITY_BYTES (1024*1024)

enum shape_type
{
    ShapeType_None,
    ShapeType_Point,
    ShapeType_Line,
    ShapeType_Triangle,
    ShapeType_NumTypes,
};

struct shape_renderer_group
{
    shape_type Type;
    uint32_t NumElements;
};

#pragma pack(push, 1)
struct shape_renderer_point
{
    float X, Y;
    float R, G, B, A;
};

struct shape_renderer_line
{
    float X1, Y1;
    float R1, G1, B1, A1;

    float X2, Y2;
    float R2, G2, B2, A2;
};

struct shape_renderer_triangle
{
    float X1, Y1;
    float R1, G1, B1, A1;

    float X2, Y2;
    float R2, G2, B2, A2;

    float X3, Y3;
    float R3, G3, B3, A3;
};
#pragma pack(pop)

struct shape_renderer
{
    uint32_t WindowWidth;
    uint32_t WindowHeight;

    uint32_t ShaderProgram;
    uint32_t Vbo;
    uint32_t Vao;

    uint8_t *VertexData;
    shape_type CurrentType;
    uint8_t *InsertionPoint;
    uint32_t NumVertices;
    float PointSize;
    float LineWidth;

    bool IsWireframe;
};

internal void
Reset(shape_renderer *ShapeRenderer)
{
    ShapeRenderer->InsertionPoint = ShapeRenderer->VertexData;
    ShapeRenderer->NumVertices = 0;
}

internal bool
Initialize(shape_renderer *ShapeRenderer, uint32_t WindowWidth, uint32_t WindowHeight)
{
    ShapeRenderer->WindowWidth = WindowWidth;
    ShapeRenderer->WindowHeight = WindowHeight;

    ShapeRenderer->VertexData = (uint8_t *)malloc(SHAPE_RENDERER_CAPACITY_BYTES);
    ShapeRenderer->CurrentType = ShapeType_None;
    ShapeRenderer->IsWireframe = false;
    Reset(ShapeRenderer);

const char *VertexShaderSource = R"STR(
    #version 330 core
    layout(location = 0) in vec2 Pos;
    layout(location = 1) in vec4 vsColor;

    uniform vec2 WindowDimensions;

    out vec4 fsColor;

    void main()
    {
        vec2 ClipPos = ((2.0f * Pos) / WindowDimensions) - 1.0f;
        gl_Position = vec4(ClipPos, 0.0, 1.0f);

        fsColor = vsColor;
    }
    )STR";

const char *FragmentShaderSource = R"STR(
    #version 330 core
    in vec4 fsColor;

    out vec4 OutputColor;

    void main()
    {
        OutputColor = fsColor;
    }
    )STR";

    ShapeRenderer->ShaderProgram = CreateShaderProgramVF(VertexShaderSource, FragmentShaderSource);
    if(ShapeRenderer->ShaderProgram == 0)
    {
        return false;
    }
    ShapeRenderer->Vbo = CreateAndBindVertexBuffer();
    ShapeRenderer->Vao = CreateAndBindVertexArray();
    SetVertexAttributeFloat(0, 2, 6, 0);
    SetVertexAttributeFloat(1, 4, 6, 2);
    return true;
}

inline internal uint32_t
GetBytesUsed(shape_renderer *ShapeRenderer)
{
    uint64_t Result = (uint64_t)(ShapeRenderer->InsertionPoint - ShapeRenderer->VertexData);
    Assert(Result == (uint32_t)Result);
    return (uint32_t)Result;
}

internal void
Flush(shape_renderer *ShapeRenderer)
{
    glBindVertexArray(ShapeRenderer->Vao);
    {
        glUseProgram(ShapeRenderer->ShaderProgram);
        int WindowDimensionsUniformLocation = glGetUniformLocation(ShapeRenderer->ShaderProgram, "WindowDimensions");
        Assert(WindowDimensionsUniformLocation != -1);
        glUniform2f(WindowDimensionsUniformLocation, (float)ShapeRenderer->WindowWidth, (float)ShapeRenderer->WindowHeight);

        FillVertexBuffer(ShapeRenderer->Vbo, ShapeRenderer->VertexData, GetBytesUsed(ShapeRenderer));

        if(ShapeRenderer->IsWireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        switch(ShapeRenderer->CurrentType)
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
        }

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    glBindVertexArray(0);

    Reset(ShapeRenderer);
}

internal void
Render(shape_renderer *ShapeRenderer)
{
    Flush(ShapeRenderer);
}

inline internal bool
HasBytesLeftFor(shape_renderer *ShapeRenderer, uint32_t Size)
{
    uint32_t BytesUsed = GetBytesUsed(ShapeRenderer);
    bool Result = (BytesUsed + Size) <= SHAPE_RENDERER_CAPACITY_BYTES;
    return Result;
}

internal void
AddPoint(shape_renderer *ShapeRenderer, v2 P, float PointSize, v4 Color)
{
    shape_renderer_point Point = {
        EXPANDV2(P), EXPANDV4(Color)
    };

    if(ShapeRenderer->CurrentType == ShapeType_Point)
    {
        if(!HasBytesLeftFor(ShapeRenderer, sizeof(Point)) ||
           ShapeRenderer->PointSize != PointSize)
        {
            Flush(ShapeRenderer);
        }
    }
    else
    {
        Flush(ShapeRenderer);

        ShapeRenderer->CurrentType = ShapeType_Point;
    }
    ShapeRenderer->PointSize = PointSize;

    *(shape_renderer_point *)ShapeRenderer->InsertionPoint = Point;
    ShapeRenderer->InsertionPoint += sizeof(Point);
    ShapeRenderer->NumVertices += 1;
}

internal void
AddLine(shape_renderer *ShapeRenderer, line2 InputLine, float LineWidth, v4 Color)
{
    shape_renderer_line Line = {
        EXPANDV2(InputLine.P1), EXPANDV4(Color), 
        EXPANDV2(InputLine.P2), EXPANDV4(Color)
    };

    if(ShapeRenderer->CurrentType == ShapeType_Line)
    {
        if(!HasBytesLeftFor(ShapeRenderer, sizeof(Line)) ||
           ShapeRenderer->LineWidth != LineWidth)
        {
            Flush(ShapeRenderer);
        }
    }
    else
    {
        Flush(ShapeRenderer);

        ShapeRenderer->CurrentType = ShapeType_Line;
    }
    ShapeRenderer->LineWidth = LineWidth;

    *(shape_renderer_line *)ShapeRenderer->InsertionPoint = Line;
    ShapeRenderer->InsertionPoint += sizeof(Line);
    ShapeRenderer->NumVertices += 2;
}

internal void
AddTriangle(shape_renderer *ShapeRenderer, tri2 Tri, v4 Color, bool IsWireframe = false)
{
    shape_renderer_triangle Triangle = {
        EXPANDV2(Tri.P1), EXPANDV4(Color), 
        EXPANDV2(Tri.P2), EXPANDV4(Color),
        EXPANDV2(Tri.P3), EXPANDV4(Color),
    };

    if(ShapeRenderer->CurrentType == ShapeType_Triangle)
    {
        if(!HasBytesLeftFor(ShapeRenderer, sizeof(Triangle)) ||
           ShapeRenderer->IsWireframe != IsWireframe)
        {
            Flush(ShapeRenderer);
        }
    }
    else
    {
        Flush(ShapeRenderer);

        ShapeRenderer->CurrentType = ShapeType_Triangle;
    }
    ShapeRenderer->IsWireframe = IsWireframe;

    *(shape_renderer_triangle *)ShapeRenderer->InsertionPoint = Triangle;
    ShapeRenderer->InsertionPoint += sizeof(Triangle);
    ShapeRenderer->NumVertices += 3;
}

internal void
AddQuad(shape_renderer *ShapeRenderer, quad2 Quad, v4 Color, bool IsWireframe = false)
{
    AddTriangle(ShapeRenderer, TRI2(Quad.BottomLeft, Quad.BottomRight, Quad.TopRight), Color, IsWireframe);
    AddTriangle(ShapeRenderer, TRI2(Quad.BottomLeft, Quad.TopRight, Quad.TopLeft), Color, IsWireframe);
}

internal void
AddRay(shape_renderer *ShapeRenderer, ray2 Ray, float Length, float LineWidth, v4 Color)
{
    v2 P1 = Ray.Pos;
    v2 P2 = Ray.Pos + Length*Ray.Dir;
    line2 Line = {P1, P2};
    AddLine(ShapeRenderer, Line, LineWidth, Color);
}

#endif
