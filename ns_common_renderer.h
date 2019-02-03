#ifndef NS_COMMON_RENDERER
#define NS_COMMON_RENDERER

#include "ns_common.h"
#include "ns_common_renderer.h"
#include "ns_opengl.h"

const char *GlobalTextureRendererVertexShaderSource = R"STR(
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

const char *GlobalTextureRendererFragmentShaderSource = R"STR(
#version 330 core
in vec2 fsTexCoord;

uniform sampler2D Texture;

out vec4 OutputColor;

void main()
{
    vec4 TexColor = texture(Texture, fsTexCoord);
    OutputColor = vec4(TexColor.r, TexColor.r, TexColor.r, 1.0f);
}
)STR";

struct common_renderer
{
    uint32_t WindowWidth;
    uint32_t WindowHeight;

    GLint ShaderProgram, PrevShaderProgram;
    GLint Vbo, PrevVbo;
    GLint Vao, PrevVao;

    uint8_t *VertexData;
};

internal common_renderer 
CreateCommonRenderer(uint32_t WindowWidth, uint32_t WindowHeight, uint32_t MaxVertexData,
                     const char *VertexShaderSource, const char *FragmentShaderSource)
{
    common_renderer Result;
    Result.WindowWidth = WindowWidth;
    Result.WindowHeight = WindowHeight;
    Result.VertexData = (uint8_t *)MemAlloc(MaxVertexData);
    Result.ShaderProgram = CreateShaderProgramVF(VertexShaderSource, FragmentShaderSource);
    return Result;
}

internal common_renderer
CreateGenericTextureCommonRenderObjects(uint32_t WindowWidth, uint32_t WindowHeight, uint32_t MaxVertexDataSize = 0)
{
    Assert(MaxVertexDataSize != 0);

    common_renderer Result = CreateCommonRenderer(WindowWidth, WindowHeight, sizeof(float)*MaxVertexDataSize,
                                                  GlobalTextureRendererVertexShaderSource, GlobalTextureRendererFragmentShaderSource);

    Result.Vbo = CreateAndBindVertexBuffer();
    Result.Vao = CreateAndBindVertexArray();
    SetVertexAttributeFloat(0, 2, 4, 0);
    SetVertexAttributeFloat(1, 2, 4, 2);
    glBindVertexArray(0);

    return Result;
}

internal void
BeginRender(common_renderer *CommonRenderer, uint32_t VertexDataNumFloats)
{
    glGetIntegerv(GL_CURRENT_PROGRAM, &CommonRenderer->PrevShaderProgram);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &CommonRenderer->PrevVao);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &CommonRenderer->PrevVbo);

    glBindVertexArray(CommonRenderer->Vao);
    glUseProgram(CommonRenderer->ShaderProgram);
    int WindowDimensionsUniformLocation = glGetUniformLocation(CommonRenderer->ShaderProgram, "WindowDimensions");
    Assert(WindowDimensionsUniformLocation != -1);
    glUniform2f(WindowDimensionsUniformLocation, (float)CommonRenderer->WindowWidth, (float)CommonRenderer->WindowHeight);
    FillVertexBuffer(CommonRenderer->Vbo, CommonRenderer->VertexData, sizeof(float)*VertexDataNumFloats);
}

internal void
EndRender(common_renderer *CommonRenderer)
{
    glUseProgram(CommonRenderer->PrevShaderProgram);
    glBindVertexArray(CommonRenderer->PrevVao);
    glBindBuffer(GL_ARRAY_BUFFER, CommonRenderer->PrevVbo);
}

internal void
Free(common_renderer *CommonRenderer)
{
    MemFree(CommonRenderer->VertexData);
}

#endif