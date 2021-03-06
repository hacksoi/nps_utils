#ifndef NS_OPENGL_H
#define NS_OPENGL_H

#include "ns_common.h"
#include "ns_opengl_functions.h"
#include "ns_game_math.h"
#include "ns_util.h"

struct render_objects
{
    uint32_t ShaderProgram;
    uint32_t Vbo, Ebo, Vao, Texture;
};

struct framebuffer
{
    uint32_t Fbo, Texture, Rbo;
};

global v4 GLUTILS_WHITE = {1.0f, 1.0f, 1.0f, 1.0f};
global v4 GLUTILS_BLACK = {0.0f, 0.0f, 0.0f, 1.0f};
global v4 GLUTILS_GRAY = ns_hex_string_to_vec("778899");
global v4 GLUTILS_RED = {1.0f, 0.0f, 0.0f, 1.0f};
global v4 GLUTILS_GREEN = {0.0f, 1.0f, 0.0f, 1.0f};
global v4 GLUTILS_BLUE = {0.0f, 0.0f, 1.0f, 1.0f};
global v4 GLUTILS_CYAN = {0.0f, 1.0f, 1.0f, 1.0f};
global v4 GLUTILS_YELLOW = {1.0f, 1.0f, 0.0f, 1.0f};
global v4 GLUTILS_ORANGE = ns_hex_string_to_vec("ffa500");
global v4 GLUTILS_PURPLE = ns_hex_string_to_vec("800080");
global v4 GLUTILS_PINK = ns_hex_string_to_vec("ff69b4");

// The default calling convention is __cdecl. Unfortunately, the OpenGL functions are defined
typedef void __stdcall opengl_get_error_info(GLuint globject, GLsizei maxLength, GLsizei *length, GLchar *infoLog); // i.e. glGetShaderInfoLog()
inline internal void
LogErrorMessageFromGLInfoFunc(uint32_t OpenGLObject, opengl_get_error_info OpenGLGetErrorInfo, const char *ExitMessage)
{
    char Buffer[512];
    OpenGLGetErrorInfo(OpenGLObject, (GLsizei)(sizeof(Buffer) - 1), NULL, Buffer);
    Log("%s: %s", ExitMessage, Buffer);
}

internal uint32_t
CreateAndBindVertexBuffer()
{
    uint32_t VertexBuffer;
    glGenBuffers(1, &VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
    return VertexBuffer;
}

internal uint32_t
CreateAndBindIndexBuffer()
{
    uint32_t IndexBuffer;
    glGenBuffers(1, &IndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
    return IndexBuffer;
}

internal uint32_t
CreateAndBindVertexArray()
{
    uint32_t VertexArray;
    glGenVertexArrays(1, &VertexArray);
    glBindVertexArray(VertexArray);
    return VertexArray;
}

internal void
FillVertexBuffer(uint32_t VertexBuffer, void *VertexData, uint32_t VertexDataSize, uint32_t Mode = GL_STREAM_DRAW)
{
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, VertexDataSize, VertexData, Mode);
}

internal void
FillIndexBuffer(uint32_t IndexBuffer, void *IndexData, uint32_t IndexDataSize, uint32_t Mode = GL_STREAM_DRAW)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexDataSize, IndexData, Mode);
}

internal uint32_t
CreateShader(GLenum ShaderType, const char *ShaderSource)
{
    uint32_t Shader = glCreateShader(ShaderType);
    glShaderSource(Shader, 1, &ShaderSource, NULL);
    glCompileShader(Shader);

    int DidCompileSuccessfully;
    glGetShaderiv(Shader, GL_COMPILE_STATUS, &DidCompileSuccessfully);
    if(!DidCompileSuccessfully)
    {
        const char *ErrorMessage;
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
        LogErrorMessageFromGLInfoFunc(Shader, glGetShaderInfoLog, ErrorMessage);
        Shader = 0;
    }
    Assert(Shader != 0);
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
            ShaderProgram = glCreateProgram();

            glAttachShader(ShaderProgram, VertexShader);
            glAttachShader(ShaderProgram, FragmentShader);
            glLinkProgram(ShaderProgram);

            int DidProgramLinkSuccessfully;
            glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &DidProgramLinkSuccessfully);
            if(!DidProgramLinkSuccessfully)
            {
                LogErrorMessageFromGLInfoFunc(ShaderProgram, glGetProgramInfoLog, "Error linking shader program: \n");
                glDeleteProgram(ShaderProgram);
                ShaderProgram = 0;
            }

            glDeleteShader(FragmentShader);
        }

        glDeleteShader(VertexShader);
    }
    Assert(ShaderProgram != 0);
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
                ShaderProgram = glCreateProgram();

                glAttachShader(ShaderProgram, VertexShader);
                glAttachShader(ShaderProgram, GeometryShader);
                glAttachShader(ShaderProgram, FragmentShader);
                glLinkProgram(ShaderProgram);

                int DidProgramLinkSuccessfully;
                glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &DidProgramLinkSuccessfully);
                if(!DidProgramLinkSuccessfully)
                {
                    LogErrorMessageFromGLInfoFunc(ShaderProgram, glGetProgramInfoLog, "Error linking shader program: \n");
                    glDeleteProgram(ShaderProgram);
                    ShaderProgram = 0;
                }

                glDeleteShader(FragmentShader);
            }

            glDeleteShader(GeometryShader);
        }

        glDeleteShader(VertexShader);
    }
    Assert(ShaderProgram != 0);
    return ShaderProgram;
}

/* Note that the VBO should already be bound. */
inline internal void
SetVertexAttributeFloat(uint32_t VertexAttributeIndex, uint32_t ElementsPerAttribute, uint32_t StrideInFloats, uint32_t OffsetInFloats)
{
    glVertexAttribPointer(VertexAttributeIndex, ElementsPerAttribute, GL_FLOAT, GL_FALSE, 
                          StrideInFloats * sizeof(float), (void *)(OffsetInFloats * sizeof(float)));
    glEnableVertexAttribArray(VertexAttributeIndex);
}

inline internal void
InsertQuad(quad2 Quad, float *VertexData, int *VertexDataCount)
{
    {
        VertexData[(*VertexDataCount)++] = Quad.BottomLeft.X;
        VertexData[(*VertexDataCount)++] = Quad.BottomLeft.Y;

        VertexData[(*VertexDataCount)++] = Quad.BottomRight.X;
        VertexData[(*VertexDataCount)++] = Quad.BottomRight.Y;

        VertexData[(*VertexDataCount)++] = Quad.TopRight.X;
        VertexData[(*VertexDataCount)++] = Quad.TopRight.Y;
    }

    {
        VertexData[(*VertexDataCount)++] = Quad.BottomLeft.X;
        VertexData[(*VertexDataCount)++] = Quad.BottomLeft.Y;
                    
        VertexData[(*VertexDataCount)++] = Quad.TopRight.X;
        VertexData[(*VertexDataCount)++] = Quad.TopRight.Y;
                    
        VertexData[(*VertexDataCount)++] = Quad.TopLeft.X;
        VertexData[(*VertexDataCount)++] = Quad.TopLeft.Y;
    }
}

internal void
InsertTexture(float *VertexData, int MaxVertexDataBytes, int *NumVertexData, quad2 Positions, float Z, quad2 TexCoord)
{
    Assert(*NumVertexData + 30 <= MaxVertexDataBytes);

    {
        VertexData[(*NumVertexData)++] = Positions.BottomLeft.X;
        VertexData[(*NumVertexData)++] = Positions.BottomLeft.Y;
        VertexData[(*NumVertexData)++] = Z;
        VertexData[(*NumVertexData)++] = TexCoord.BottomLeft.X;
        VertexData[(*NumVertexData)++] = TexCoord.BottomLeft.Y;

        VertexData[(*NumVertexData)++] = Positions.BottomRight.X;
        VertexData[(*NumVertexData)++] = Positions.BottomRight.Y;
        VertexData[(*NumVertexData)++] = Z;
        VertexData[(*NumVertexData)++] = TexCoord.BottomRight.X;
        VertexData[(*NumVertexData)++] = TexCoord.BottomRight.Y;

        VertexData[(*NumVertexData)++] = Positions.TopLeft.X;
        VertexData[(*NumVertexData)++] = Positions.TopLeft.Y;
        VertexData[(*NumVertexData)++] = Z;
        VertexData[(*NumVertexData)++] = TexCoord.TopLeft.X;
        VertexData[(*NumVertexData)++] = TexCoord.TopLeft.Y;
    }

    {
        VertexData[(*NumVertexData)++] = Positions.BottomRight.X;
        VertexData[(*NumVertexData)++] = Positions.BottomRight.Y;
        VertexData[(*NumVertexData)++] = Z;
        VertexData[(*NumVertexData)++] = TexCoord.BottomRight.X;
        VertexData[(*NumVertexData)++] = TexCoord.BottomRight.Y;

        VertexData[(*NumVertexData)++] = Positions.TopRight.X;
        VertexData[(*NumVertexData)++] = Positions.TopRight.Y;
        VertexData[(*NumVertexData)++] = Z;
        VertexData[(*NumVertexData)++] = TexCoord.TopRight.X;
        VertexData[(*NumVertexData)++] = TexCoord.TopRight.Y;

        VertexData[(*NumVertexData)++] = Positions.TopLeft.X;
        VertexData[(*NumVertexData)++] = Positions.TopLeft.Y;
        VertexData[(*NumVertexData)++] = Z;
        VertexData[(*NumVertexData)++] = TexCoord.TopLeft.X;
        VertexData[(*NumVertexData)++] = TexCoord.TopLeft.Y;
    }
}

internal void
InsertTexture(float *VertexData, int MaxVertexDataBytes, int *NumVertexData, rect2 PosRect, float Z, rect2 TexCoordRect)
{
    quad2 PosQuad = QUAD2(PosRect);
    quad2 TexCoordQuad = QUAD2(TexCoordRect);
    InsertTexture(VertexData, MaxVertexDataBytes, NumVertexData, PosQuad, Z, TexCoordQuad);
}

internal void
TurnOnWireFrame()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

internal void
TurnOffWireFrame()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

inline internal void
EnableSmoothLines()
{
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

void SetUniform(int ShaderProgram, const char *UniformName, float X, float Y)
{
    int UniformLocation = glGetUniformLocation(ShaderProgram, UniformName);
    Assert(UniformLocation != -1);
    glUniform2f(UniformLocation, X, Y);
}

void SetUniform(int ShaderProgram, const char *UniformName, float X)
{
    int UniformLocation = glGetUniformLocation(ShaderProgram, UniformName);
    Assert(UniformLocation != -1);
    glUniform1f(UniformLocation, X);
}

void SetUniform(int ShaderProgram, const char *UniformName, int X, bool OkayIfDoesntExist = false)
{
    int UniformLocation = glGetUniformLocation(ShaderProgram, UniformName);
    if (UniformLocation != -1)
    {
        glUniform1i(UniformLocation, X);
    }
    else
    {
        Assert(OkayIfDoesntExist);
    }
}

#endif
