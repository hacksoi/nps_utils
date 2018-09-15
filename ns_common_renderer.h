#ifndef NS_COMMON_RENDERER
#define NS_COMMON_RENDERER

#include "ns_common.h"

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

internal void
BeginRender(common_renderer *CommonRenderer, uint32_t SizeOfVertexData)
{
    glGetIntegerv(GL_CURRENT_PROGRAM, &CommonRenderer->PrevShaderProgram);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &CommonRenderer->PrevVao);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &CommonRenderer->PrevVbo);

    glBindVertexArray(CommonRenderer->Vao);
    glUseProgram(CommonRenderer->ShaderProgram);
    int WindowDimensionsUniformLocation = glGetUniformLocation(CommonRenderer->ShaderProgram, "WindowDimensions");
    Assert(WindowDimensionsUniformLocation != -1);
    glUniform2f(WindowDimensionsUniformLocation, (float)CommonRenderer->WindowWidth, (float)CommonRenderer->WindowHeight);
    FillVertexBuffer(CommonRenderer->Vbo, CommonRenderer->VertexData, SizeOfVertexData);
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