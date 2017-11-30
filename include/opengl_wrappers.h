#ifndef OPENGL_WRAPPERS_H
#define OPENGL_WRAPPERS_H

#include "glutils_error.h"
#include "opengl_functions.h"

struct render_objects
{
    uint32_t ShaderProgram;
    uint32_t Vbo, Ebo, Vao;
};

internal uint32_t
CreateAndBindVertexBuffer()
{
    uint32_t VertexBuffer;
    glGenBuffers(1, &VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
    return VertexBuffer;
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

internal uint32_t
CreateShader(GLenum ShaderType, const char *ShaderSource)
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
            ShaderProgram = glCreateProgram();

            glAttachShader(ShaderProgram, VertexShader);
            glAttachShader(ShaderProgram, FragmentShader);
            glLinkProgram(ShaderProgram);

            bool32 DidProgramLinkSuccessfully;
            glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &DidProgramLinkSuccessfully);
            if(!DidProgramLinkSuccessfully)
            {
                SetErrorMessageFromGLInfoFunc(ShaderProgram, glGetProgramInfoLog, 
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
                ShaderProgram = glCreateProgram();

                glAttachShader(ShaderProgram, VertexShader);
                glAttachShader(ShaderProgram, GeometryShader);
                glAttachShader(ShaderProgram, FragmentShader);
                glLinkProgram(ShaderProgram);

                bool32 DidProgramLinkSuccessfully;
                glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &DidProgramLinkSuccessfully);
                if(!DidProgramLinkSuccessfully)
                {
                    SetErrorMessageFromGLInfoFunc(ShaderProgram, glGetProgramInfoLog, 
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

inline internal void
SetVertexAttributeFloat(uint32_t VertexAttributeIndex, uint32_t ElementsPerAttribute, uint32_t StrideInFloats, uint32_t OffsetInFloats)
{
    glVertexAttribPointer(VertexAttributeIndex, ElementsPerAttribute, GL_FLOAT, GL_FALSE, 
                          StrideInFloats * sizeof(float), (void *)(OffsetInFloats * sizeof(float)));
    glEnableVertexAttribArray(VertexAttributeIndex);
}

#endif
