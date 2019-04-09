#ifndef NS_COMMON_RENDERER
#define NS_COMMON_RENDERER

#include "ns_common.h"
#include "ns_memory.h"
#include "ns_common_renderer.h"
#include "ns_opengl.h"

struct common_renderer
{
    int WindowWidth;
    int WindowHeight;

    GLint ShaderProgram, PrevShaderProgram;
    GLint Vbo, PrevVbo;
    GLint Vao, PrevVao;

    uint8_t *VertexData;
    int NumVertexData;
    int MaxVertexDataBytes;

    v2 CameraPos;
    float Zoom;
    int FragShaderDebugValue;
};

internal common_renderer
CreateCommonRenderer(int WindowWidth, int WindowHeight, int MaxVertexDataBytes,
                     const char *VertexShaderSource, const char *FragmentShaderSource)
{
    common_renderer Result = {};
    Result.WindowWidth = WindowWidth;
    Result.WindowHeight = WindowHeight;
    Result.VertexData = (uint8_t *)MemAlloc(MaxVertexDataBytes);
    Result.MaxVertexDataBytes = MaxVertexDataBytes;
    Result.ShaderProgram = CreateShaderProgramVF(VertexShaderSource, FragmentShaderSource);
    Result.CameraPos = V2((float)WindowWidth/2.0f, (float)WindowHeight/2.0f);
    Result.Zoom = 1.0f;
    return Result;
}

internal void
BeginRender(common_renderer *CommonRenderer, int VertexDataNumFloats)
{
    glGetIntegerv(GL_CURRENT_PROGRAM, &CommonRenderer->PrevShaderProgram);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &CommonRenderer->PrevVao);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &CommonRenderer->PrevVbo);

    glBindVertexArray(CommonRenderer->Vao);
    glUseProgram(CommonRenderer->ShaderProgram);

    SetUniform(CommonRenderer->ShaderProgram, "WindowDimensions", (float)CommonRenderer->WindowWidth, (float)CommonRenderer->WindowHeight);
    SetUniform(CommonRenderer->ShaderProgram, "CameraPos", (float)CommonRenderer->CameraPos.X, (float)CommonRenderer->CameraPos.Y);
    SetUniform(CommonRenderer->ShaderProgram, "Zoom", CommonRenderer->Zoom);
    SetUniform(CommonRenderer->ShaderProgram, "fsDebugValue", CommonRenderer->FragShaderDebugValue, true);

    Assert((int)sizeof(float)*VertexDataNumFloats < CommonRenderer->MaxVertexDataBytes);
    FillVertexBuffer(CommonRenderer->Vbo, CommonRenderer->VertexData, sizeof(float)*VertexDataNumFloats);
}

internal void
EndRender(common_renderer *CommonRenderer)
{
    glUseProgram(CommonRenderer->PrevShaderProgram);
    glBindVertexArray(CommonRenderer->PrevVao);
    glBindBuffer(GL_ARRAY_BUFFER, CommonRenderer->PrevVbo);
}

void SetWindowSize(common_renderer *CommonRenderer, int NewWindowWidth, int NewWindowHeight)
{
    CommonRenderer->WindowHeight = NewWindowHeight;
    CommonRenderer->WindowWidth = NewWindowWidth;

    /* Let's make the bad decision of being clever and changing the camera pos too... */
    v2 NewCameraPos = V2((float)NewWindowWidth/2.0f, (float)NewWindowHeight/2.0f);
    CommonRenderer->CameraPos = NewCameraPos;
}

internal void
Free(common_renderer *CommonRenderer)
{
    MemFree(CommonRenderer->VertexData);
}

#endif