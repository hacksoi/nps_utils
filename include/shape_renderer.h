#ifndef SHAPE_RENDERER_H
#define SHAPE_RENDERER_H

#include "common.h"
#include "opengl_functions.h"

#define SHAPE_RENDERER_CAPACITY_BYTES 8192

enum shape_type
{
    ShapeType_None,
    ShapeType_Point,
    ShapeType_Line,
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
};

internal void
Reset(shape_renderer *ShapeRenderer)
{
    ShapeRenderer->InsertionPoint = ShapeRenderer->VertexData;
    ShapeRenderer->NumVertices = 0;
}

internal bool32
Initialize(shape_renderer *ShapeRenderer, uint32_t WindowWidth, uint32_t WindowHeight)
{
    ShapeRenderer->WindowWidth = WindowWidth;
    ShapeRenderer->WindowHeight = WindowHeight;

    ShapeRenderer->VertexData = (uint8_t *)malloc(SHAPE_RENDERER_CAPACITY_BYTES);
    ShapeRenderer->CurrentType = ShapeType_None;
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
    glUseProgram(ShapeRenderer->ShaderProgram);
    glBindVertexArray(ShapeRenderer->Vao);

    int WindowDimensionsUniformLocation = glGetUniformLocation(ShapeRenderer->ShaderProgram, "WindowDimensions");
    Assert(WindowDimensionsUniformLocation != -1);
    glUniform2f(WindowDimensionsUniformLocation, (float)ShapeRenderer->WindowWidth, (float)ShapeRenderer->WindowHeight);

    FillVertexBuffer(ShapeRenderer->Vbo, ShapeRenderer->VertexData, GetBytesUsed(ShapeRenderer));

    switch(ShapeRenderer->CurrentType)
    {
        case ShapeType_Point:
        {
            glPointSize(7.0f);
            glDrawArrays(GL_POINTS, 0, ShapeRenderer->NumVertices);
        } break;

        case ShapeType_Line:
        {
            glLineWidth(3.0f);
            glDrawArrays(GL_LINES, 0, ShapeRenderer->NumVertices);
        } break;
    }

    Reset(ShapeRenderer);
}

internal void
Render(shape_renderer *ShapeRenderer)
{
    Flush(ShapeRenderer);
}

inline internal bool32
HasBytesLeftFor(shape_renderer *ShapeRenderer, uint32_t Size)
{
    uint32_t BytesUsed = GetBytesUsed(ShapeRenderer);
    bool32 Result = (BytesUsed + Size) < SHAPE_RENDERER_CAPACITY_BYTES;
    return Result;
}

internal void
AddPoint(shape_renderer *ShapeRenderer, v2 P, v4 Color)
{
    shape_renderer_point Point = {EXPANDV2(P), EXPANDV4(Color)};

    if(ShapeRenderer->CurrentType == ShapeType_Point)
    {
        if(!HasBytesLeftFor(ShapeRenderer, sizeof(Point)))
        {
            Flush(ShapeRenderer);
        }
    }
    else
    {
        Flush(ShapeRenderer);

        ShapeRenderer->CurrentType = ShapeType_Point;
    }

    *(shape_renderer_point *)ShapeRenderer->InsertionPoint = Point;
    ShapeRenderer->InsertionPoint += sizeof(Point);
    ShapeRenderer->NumVertices += 1;
}

internal void
AddRay(shape_renderer *ShapeRenderer, ray2 Ray, float Length, v4 Color)
{
    v2 P1 = Ray.Pos;
    v2 P2 = Ray.Pos + Length*Ray.Dir;
    shape_renderer_line Line = {EXPANDV2(P1), EXPANDV4(Color), EXPANDV2(P2), EXPANDV4(Color)};

    if(ShapeRenderer->CurrentType == ShapeType_Line)
    {
        if(!HasBytesLeftFor(ShapeRenderer, sizeof(Line)))
        {
            Flush(ShapeRenderer);
        }
    }
    else
    {
        Flush(ShapeRenderer);

        ShapeRenderer->CurrentType = ShapeType_Line;
    }

    *(shape_renderer_line *)ShapeRenderer->InsertionPoint = Line;
    ShapeRenderer->InsertionPoint += sizeof(Line);
    ShapeRenderer->NumVertices += 2;
}

#endif
