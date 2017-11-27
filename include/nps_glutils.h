/* Includes */
//{
#include <string.h>

#include "nps_common_defs.h"
#include "nps_math.h"
//}

/* Error Messages Handling */
//{
internal char npsGlutilsErrorBuffer[512];

inline internal char *
GetGlutilsErrorMessage()
{
    return npsGlutilsErrorBuffer;
}

typedef void opengl_get_error_info(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog); // i.e. glGetShaderInfoLog()
inline internal void
SetErrorMessageFromGLInfoFunc(uint32_t OpenGLObject, opengl_get_error_info OpenGLGetErrorInfo, char *ExitMessage)
{
    size_t ExitMessageLength = strlen(ExitMessage);
    strcpy(GeneralBuffer, ExitMessage);
    OpenGLGetErrorInfo(OpenGLObject, (GLsizei)(sizeof(npsGlutilsErrorBuffer) - ExitMessageLength - 1), 
                       NULL, npsGlutilsErrorBuffer + ExitMessageLength);
}
//}

/* Misc */
//{
struct render_objects
{
    uint32_t ShaderProgram;
    uint32_t Vbo, Ebo, Vao;
};

v4 NPS_RED = {1.0f, 0.0f, 0.0f, 1.0f};
v4 NPS_GREEN = {0.0f, 1.0f, 0.0f, 1.0f};
v4 NPS_BLUE = {0.0f, 0.0f, 1.0f, 1.0f};
v4 NPS_YELLOW = {1.0f, 1.0f, 0.0f, 1.0f};
v4 NPS_ORANGE = {1.0f, 0.63f, 0.48f, 1.0f};
//}

/* OpenGL Function Loader */
//{

/* OpenGL function pointers. */
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLCLEARCOLORPROC glClearColor;
PFNGLCLEARPROC glClear;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLDRAWELEMENTSPROC glDrawElements;
PFNGLDRAWARRAYSPROC glDrawArrays;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLUNIFORM1UIPROC glUniform1ui;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM2IPROC glUniform2i;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLVIEWPORTPROC glViewport;
PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
PFNGLGENTEXTURESPROC glGenTextures;
PFNGLBINDTEXTUREPROC glBindTexture;
PFNGLTEXIMAGE2DPROC glTexImage2D;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
PFNGLTEXPARAMETERIPROC glTexParameteri;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLENABLEPROC glEnable;
PFNGLDISABLEPROC glDisable;
PFNGLFRONTFACEPROC glFrontFace;
PFNGLBLENDFUNCPROC glBlendFunc;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLLINEWIDTHPROC glLineWidth;
PFNGLBLENDEQUATIONPROC glBlendEquation;
PFNGLPOINTSIZEPROC glPointSize;
PFNGLPOLYGONMODEPROC glPolygonMode;

/* Attempts to load the specified OpenGL function.
 * Returns 0 on failure. */
internal void *
GetGLFunctionAddress(const char *name)
{
    /* wglGetProcAddress() returns addresses for all OpenGL functions except
     * ones exported by OpenGL32.dll. */
    void *p = (void *)wglGetProcAddress(name);

    /* Did it fail? */
    if(p == 0 || (p == (void *)0x1) || (p == (void *)0x2) || (p == (void *)0x3) || (p == (void *)-1))
    {
        /* Try loading from OpengGL32.dll directly. */

        HMODULE module = LoadLibraryA("opengl32.dll");
        p = (void *)GetProcAddress(module, name);

        if(p == 0)
        {
            SetGlutilsErrorMessage("could not load opengl function: %s\n", name);
        }
    }

    return p;
}

/* Initializes the OpenGL functions.
 * Returns whether it was successful or not. */
internal bool32
LoadOpenGLFunctions()
{
    if(
       /* Extensions */
       (wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)GetGLFunctionAddress("wglChoosePixelFormatARB")) == (void *)0 ||
       (wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)GetGLFunctionAddress("wglCreateContextAttribsARB")) == (void *)0 ||

       /* Core OpenGL */
       (glAttachShader = (PFNGLATTACHSHADERPROC)GetGLFunctionAddress("glAttachShader")) == (void *)0 ||
       (glBindBuffer = (PFNGLBINDBUFFERPROC)GetGLFunctionAddress("glBindBuffer")) == (void *)0 ||
       (glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)GetGLFunctionAddress("glBindVertexArray")) == (void *)0 ||
       (glBufferData = (PFNGLBUFFERDATAPROC)GetGLFunctionAddress("glBufferData")) == (void *)0 ||
       (glClearColor = (PFNGLCLEARCOLORPROC)GetGLFunctionAddress("glClearColor")) == (void *)0 ||
       (glClear = (PFNGLCLEARPROC)GetGLFunctionAddress("glClear")) == (void *)0 ||
       (glCompileShader = (PFNGLCOMPILESHADERPROC)GetGLFunctionAddress("glCompileShader")) == (void *)0 ||
       (glCreateProgram = (PFNGLCREATEPROGRAMPROC)GetGLFunctionAddress("glCreateProgram")) == (void *)0 ||
       (glCreateShader = (PFNGLCREATESHADERPROC)GetGLFunctionAddress("glCreateShader")) == (void *)0 ||
       (glDeleteShader = (PFNGLDELETESHADERPROC)GetGLFunctionAddress("glDeleteShader")) == (void *)0 ||
       (glDrawElements = (PFNGLDRAWELEMENTSPROC)GetGLFunctionAddress("glDrawElements")) == (void *)0 ||
       (glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)GetGLFunctionAddress("glEnableVertexAttribArray")) == (void *)0 ||
       (glGenBuffers = (PFNGLGENBUFFERSPROC)GetGLFunctionAddress("glGenBuffers")) == (void *)0 ||
       (glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)GetGLFunctionAddress("glGenVertexArrays")) == (void *)0 ||
       (glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)GetGLFunctionAddress("glGetProgramInfoLog")) == (void *)0 ||
       (glGetProgramiv = (PFNGLGETPROGRAMIVPROC)GetGLFunctionAddress("glGetProgramiv")) == (void *)0 ||
       (glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)GetGLFunctionAddress("glGetShaderInfoLog")) == (void *)0 ||
       (glGetShaderiv = (PFNGLGETSHADERIVPROC)GetGLFunctionAddress("glGetShaderiv")) == (void *)0 ||
       (glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)GetGLFunctionAddress("glGetUniformLocation")) == (void *)0 ||
       (glUniform1ui = (PFNGLUNIFORM1UIPROC)GetGLFunctionAddress("glUniform1ui")) == (void *)0 ||
       (glUniform2f = (PFNGLUNIFORM2FPROC)GetGLFunctionAddress("glUniform2f")) == (void *)0 ||
       (glUniform2i = (PFNGLUNIFORM2IPROC)GetGLFunctionAddress("glUniform2i")) == (void *)0 ||
       (glLinkProgram = (PFNGLLINKPROGRAMPROC)GetGLFunctionAddress("glLinkProgram")) == (void *)0 ||
       (glShaderSource = (PFNGLSHADERSOURCEPROC)GetGLFunctionAddress("glShaderSource")) == (void *)0 ||
       (glUseProgram = (PFNGLUSEPROGRAMPROC)GetGLFunctionAddress("glUseProgram")) == (void *)0 ||
       (glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)GetGLFunctionAddress("glVertexAttribPointer")) == (void *)0 ||
       (glViewport = (PFNGLVIEWPORTPROC)GetGLFunctionAddress("glViewport")) == (void *)0 ||
       (glDrawArrays = (PFNGLDRAWARRAYSPROC)GetGLFunctionAddress("glDrawArrays")) == (void *)0 ||
       (glGenTextures = (PFNGLGENTEXTURESPROC)GetGLFunctionAddress("glGenTextures")) == (void *)0 ||
       (glBindTexture = (PFNGLBINDTEXTUREPROC)GetGLFunctionAddress("glBindTexture")) == (void *)0 ||
       (glTexImage2D = (PFNGLTEXIMAGE2DPROC)GetGLFunctionAddress("glTexImage2D")) == (void *)0 ||
       (glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)GetGLFunctionAddress("glGenerateMipmap")) == (void *)0 ||
       (glTexParameteri = (PFNGLTEXPARAMETERIPROC)GetGLFunctionAddress("glTexParameteri")) == (void *)0 ||
       (glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)GetGLFunctionAddress("glUniformMatrix4fv")) == (void *)0 ||
       (glFrontFace = (PFNGLFRONTFACEPROC)GetGLFunctionAddress("glFrontFace")) == (void *)0 ||
       (glEnable = (PFNGLENABLEPROC)GetGLFunctionAddress("glEnable")) == (void *)0 ||
       (glDisable = (PFNGLDISABLEPROC)GetGLFunctionAddress("glDisable")) == (void *)0 ||
       (glBlendFunc = (PFNGLBLENDFUNCPROC)GetGLFunctionAddress("glBlendFunc")) == (void *)0 ||
       (glUniform1f = (PFNGLUNIFORM1FPROC)GetGLFunctionAddress("glUniform1f")) == (void *)0 ||
       (glLineWidth = (PFNGLLINEWIDTHPROC)GetGLFunctionAddress("glLineWidth")) == (void *)0 ||
       (glBlendEquation = (PFNGLBLENDEQUATIONPROC)GetGLFunctionAddress("glBlendEquation")) == (void *)0 ||
       (glPointSize = (PFNGLPOINTSIZEPROC)GetGLFunctionAddress("glPointSize")) == (void *)0 ||
       (glPolygonMode = (PFNGLPOLYGONMODEPROC)GetGLFunctionAddress("glPolygonMode")) == (void *)0
       )
    {
        return false;
    }

    return true;
}
//}

/* Shaders and Programs */
//{
internal uint32_t
CreateShader(GLenum ShaderType, char *ShaderSource)
{
    uint32_t Shader = glCreateShader(ShaderType);
    glShaderSource(Shader, 1, &ShaderSource, NULL);
    glCompileShader(Shader);

    bool32 DidCompileSuccessfully;
    glGetShaderiv(Shader, GL_COMPILE_STATUS, &DidCompileSuccessfully);
    if(!DidCompileSuccessfully)
    {
        char *ErrorMessage;
        switch(ShaderType)
        {
            case GL_VERTEX_SHADER:
            {
                ErrorMessage = "error compiling vertex shader: \n";
            } break;

            case GL_FRAGMENT_SHADER:
            {
                ErrorMessage = "error compiling fragment shader: \n";
            } break;

            case GL_GEOMETRY_SHADER:
            {
                ErrorMessage = "error compiling geometry shader: \n";
            } break;

            default:
            {
                ErrorMessage = "error compiling unknown shader type: \n";
            } break;
        }
        SetErrorMessageFromGLInfoFunc(Shader, glGetShaderInfoLog, ErrorMessage);

        Shader = 0;
    }

    return Shader;
}

internal uint32_t
CreateShaderProgramVF(const char *VertexShaderSource, const char *FragmentShaderSource)
{
    uint32_t ShaderProgram = 0;

    uint32_t VertexShader = CreateShader(GL_VERTEX_SHADER, VertexShaderSource);
    if(VertexShader != 0)
    {
        uint32_t FragmentShader = CreateShader(GL_FRAGMENT_SHADER, FragmentShaderSource);
        if(FragmentShader != 0)
        {
            uint32_t ShaderProgram = glCreateProgram();

            glAttachShader(ShaderProgram, VertexShader);
            glAttachShader(ShaderProgram, FragmentShader);
            glLinkProgram(ShaderProgram);

            bool32 DidProgramLinkSuccessfully;
            glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &DidProgramLinkSuccessfully);
            if(!DidProgramLinkSuccessfully)
            {
                SetErrorMessageFromGLInfoFunc(AARenderObjects.ShaderProgram, glGetProgramInfoLog, 
                                              "Error linking shader program: \n");

                glDeleteProgram(ShaderProgram);
                ShaderProgram = 0;
            }

            glDeleteShader(FragmentShader);
        }

        glDeleteShader(VertexShader);
    }

    return ShaderProgram;
}

internal uint32_t
CreateShaderProgramVGF(const char *VertexShaderSource, const char *GeometryShaderSource, const char *FragmentShaderSource)
{
    uint32_t ShaderProgram = 0;

    uint32_t VertexShader = CreateShader(GL_VERTEX_SHADER, VertexShaderSource);
    if(VertexShader != 0)
    {
        uint32_t GeometryShader = CreateShader(GL_GEOMETRY_SHADER, GeometryShaderSource);
        if(GeometryShader != 0)
        {
            uint32_t FragmentShader = CreateShader(GL_FRAGMENT_SHADER, FragmentShaderSource);
            if(FragmentShader != 0)
            {
                uint32_t ShaderProgram = glCreateProgram();

                glAttachShader(ShaderProgram, VertexShader);
                glAttachShader(ShaderProgram, GeometryShader);
                glAttachShader(ShaderProgram, FragmentShader);
                glLinkProgram(ShaderProgram);

                bool32 DidProgramLinkSuccessfully;
                glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &DidProgramLinkSuccessfully);
                if(!DidProgramLinkSuccessfully)
                {
                    SetErrorMessageFromGLInfoFunc(AARenderObjects.ShaderProgram, glGetProgramInfoLog, 
                                                  "Error linking shader program: \n");

                    glDeleteProgram(ShaderProgram);

                    ShaderProgram = 0;
                }

                glDeleteShader(FragmentShader);
            }

            glDeleteShader(GeometryShader);
        }

        glDeleteShader(VertexShader);
    }

    return ShaderProgram;
}
//}

/* Shape Renderer */
//{
#define SR_MAX_POINTS 64
#define SR_MAX_LINES 64
struct shape_renderer
{
    uint32_t ShaderProgram;
    uint32_t Vbo;

    float PointData[SR_MAX_POINTS];
    float LineData[SR_MAX_LINES*4];
};

internal shape_renderer
IntializeShapeRenderer()
{
    const char *VertexShaderSource = R"STR(
#version 330 core
layout(location = 0) in vec2 Pos;
layout(location = 1) in vec4 vsColor;

uniform vec2 WindowDimensions;

out vec4 fsColor;

void
main()
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

void
main()
{
    OutputColor = fsColor;
}
)STR";

    shape_renderer Result = {};

    Result.ShaderProgram = CreateShaderProgramVF(VertexShaderSource, FragmentShaderSource);

    glGenBuffers(1, &Result.Vbo);
    glBindBuffer(GL_ARRAY_BUFFER, Result.Vbo);

    glGenVertexArrays(1, &Result.Vao);
    glBindVertexArray(Result.Vao);
}

internal void
AddPoint(v2 P, v4 Color)
{
    float VertexData[] = {
        EXPANDV2(P), EXPANDV4(Color),
    };

    glBindBuffer(GL_ARRAY_BUFFER, BasicColorRenderObjects.Vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_DYNAMIC_DRAW);

    glPointSize(7.5f);

    glUseProgram(BasicColorRenderObjects.ShaderProgram);
    glUniform2f(GetUniformLocation(BasicColorRenderObjects.ShaderProgram, "WindowDimensions"), (float)npsWindowWidth, (float)npsWindowHeight);
    glBindVertexArray(BasicColorRenderObjects.Vao);
    glDrawArrays(GL_POINTS, 0, 1);
}

internal void
AddRay(ray2 Ray, float Length, v4 Color)
{
    line2 Line = {Ray.Pos, Ray.Pos + Length*Ray.Dir};

    float VertexData[] = {
        EXPANDV2(Line.P1), EXPANDV4(Color),
        EXPANDV2(Line.P2), EXPANDV4(Color),
    };

    glBindBuffer(GL_ARRAY_BUFFER, BasicColorRenderObjects.Vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_DYNAMIC_DRAW);

    glUseProgram(BasicColorRenderObjects.ShaderProgram);
    glUniform2f(GetUniformLocation(BasicColorRenderObjects.ShaderProgram, "WindowDimensions"), (float)npsWindowWidth, (float)npsWindowHeight);
    glBindVertexArray(BasicColorRenderObjects.Vao);
    glDrawArrays(GL_LINES, 0, 2);
}

internal void
RenderShapes(shape_renderer *ShapeRenderer)
{
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}
//}
