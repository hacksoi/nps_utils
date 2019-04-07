#ifndef NS_COMMON_RENDERER
#define NS_COMMON_RENDERER

#include "ns_common.h"
#include "ns_memory.h"
#include "ns_common_renderer.h"
#include "ns_opengl.h"

const char *GlobalTextureRendererVertexShaderSource = R"STR(
#version 330 core
layout (location = 0) in vec2 Pos;
layout (location = 1) in vec2 vsTexCoord;

uniform float Zoom;
uniform vec2 WindowDimensions;
uniform vec2 CameraPos;

out vec2 fsTexCoord;

void main()
{
    vec2 ViewSpacePos = Pos - CameraPos;
    ViewSpacePos *= Zoom;
    ViewSpacePos += 0.5f*WindowDimensions;
#if 1
    vec2 SnappedViewSpacePos = floor(ViewSpacePos);
    vec2 ClipPos = ((2.0f * SnappedViewSpacePos) / WindowDimensions) - 1.0f;
#else
    vec2 ClipPos = ((2.0f * ViewSpacePos) / WindowDimensions) - 1.0f;
#endif
    gl_Position = vec4(ClipPos, 0.0, 1.0f);

    fsTexCoord = vsTexCoord;
}
)STR";

const char *GlobalTextureRendererFragmentShaderSource = R"STR(
#version 330 core
in vec2 fsTexCoord;

uniform sampler2D Texture;

uniform int fsDebugValue;

out vec4 OutputColor;

void main()
{
    vec4 TexColor = texture(Texture, fsTexCoord);
    OutputColor = vec4(TexColor.r, TexColor.g, TexColor.b, TexColor.a);
}
)STR";

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

internal common_renderer
CreateGenericTextureCommonRenderObjects(int WindowWidth, int WindowHeight, int MaxVertexDataBytes, 
                                        const char *VertexShaderSource, const char *FragmentShaderSource)
{
    common_renderer Result = CreateCommonRenderer(WindowWidth, WindowHeight, MaxVertexDataBytes,
                                                  VertexShaderSource, FragmentShaderSource);

    Result.Vbo = CreateAndBindVertexBuffer();
    Result.Vao = CreateAndBindVertexArray();
    SetVertexAttributeFloat(0, 2, 4, 0);
    SetVertexAttributeFloat(1, 2, 4, 2);
    glBindVertexArray(0);

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