#ifndef GLUTILS_ERROR_H
#define GLUTILS_ERROR_H

#include <string.h>

internal char GlutilsErrorBuffer[512];

inline internal char *
GetGlutilsErrorMessage()
{
    return GlutilsErrorBuffer;
}

inline internal void
SetGlutilsErrorMessage(char *Format, ...)
{
    va_list VarArgs;
    va_start(VarArgs, Format);
    vsprintf(GlutilsErrorBuffer, Format, VarArgs);
    va_end(VarArgs);
}

typedef void opengl_get_error_info(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog); // i.e. glGetShaderInfoLog()
inline internal void
SetErrorMessageFromGLInfoFunc(uint32_t OpenGLObject, opengl_get_error_info OpenGLGetErrorInfo, char *ExitMessage)
{
    size_t ExitMessageLength = strlen(ExitMessage);
    strcpy(GlutilsErrorBuffer, ExitMessage);
    OpenGLGetErrorInfo(OpenGLObject, (GLsizei)(sizeof(GlutilsErrorBuffer) - ExitMessageLength - 1), 
                       NULL, GlutilsErrorBuffer + ExitMessageLength);
}

#endif
