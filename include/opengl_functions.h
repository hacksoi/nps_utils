#ifndef OPENGL_FUNCTIONS_H
#define OPENGL_FUNCTIONS_H

#include "glcorearb.h"

#include "common.h"
#include "glutils_error.h"

PFNGLATTACHSHADERPROC glAttachShader;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLCLEARCOLORPROC glClearColor;
PFNGLCLEARPROC glClear;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLDELETESHADERPROC glDeleteProgram;
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

internal void *
GetGLFunctionAddress(const char *FunctionName)
{
    void *FunctionAddress = (void *)wglGetProcAddress(FunctionName);
    bool32 DidLoadFail = ((FunctionAddress == 0) || 
                          (FunctionAddress == (void *)0x1) || 
                          (FunctionAddress == (void *)0x2) || 
                          (FunctionAddress == (void *)0x3) || 
                          (FunctionAddress == (void *)-1));
    if(DidLoadFail)
    {
        HMODULE OpenGLDLLHandle = LoadLibraryA("opengl32.dll");
        FunctionAddress = (void *)GetProcAddress(OpenGLDLLHandle, FunctionName);
        if(FunctionAddress == 0)
        {
            SetGlutilsErrorMessage("could not load opengl function: %s\n", FunctionName);
        }
    }
    return FunctionAddress;
}

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
       (glPolygonMode = (PFNGLPOLYGONMODEPROC)GetGLFunctionAddress("glPolygonMode")) == (void *)0 ||
       (glDeleteProgram = (PFNGLDELETEPROGRAMPROC)GetGLFunctionAddress("glDeleteProgram")) == (void *)0
       )
    {
        return false;
    }
    return true;
}

#endif
