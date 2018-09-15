#ifndef NS_OPENGL_FUNCTIONS_H
#define NS_OPENGL_FUNCTIONS_H

#include "glcorearb.h"
#include "wglext.h"

#include "ns_common.h"

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
PFNGLUNIFORM1IPROC glUniform1i;
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
PFNGLHINTPROC glHint;
PFNGLGETINTEGERVPROC glGetIntegerv;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLISENABLEDPROC glIsEnabled;
PFNGLSCISSORPROC glScissor;
PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate;
PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLGETERRORPROC glGetError;
PFNGLDELETETEXTURESPROC glDeleteTextures;

internal void *
GetGLFunctionAddress(const char *FunctionName)
{
    void *FunctionAddress = (void *)wglGetProcAddress(FunctionName);
    bool DidLoadFail = ((FunctionAddress == 0) || 
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
            Log("could not load opengl function: %s\n", FunctionName);
        }
    }
    return FunctionAddress;
}

internal bool
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
       (glDeleteProgram = (PFNGLDELETEPROGRAMPROC)GetGLFunctionAddress("glDeleteProgram")) == (void *)0 ||
       (glUniform1i = (PFNGLUNIFORM1IPROC)GetGLFunctionAddress("glUniform1i")) == (void *)0 ||
       (glHint = (PFNGLHINTPROC)GetGLFunctionAddress("glHint")) == (void *)0 ||
       (glGetIntegerv = (PFNGLGETINTEGERVPROC)GetGLFunctionAddress("glGetIntegerv")) == (void *)0 ||
       (glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)GetGLFunctionAddress("glGetAttribLocation")) == (void *)0 ||
       (glActiveTexture = (PFNGLACTIVETEXTUREPROC)GetGLFunctionAddress("glActiveTexture")) == (void *)0 ||
       (glIsEnabled = (PFNGLISENABLEDPROC)GetGLFunctionAddress("glIsEnabled")) == (void *)0 ||
       (glScissor = (PFNGLSCISSORPROC)GetGLFunctionAddress("glScissor")) == (void *)0 ||
       (glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC)GetGLFunctionAddress("glBlendEquationSeparate")) == (void *)0 ||
       (glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)GetGLFunctionAddress("glBlendFuncSeparate")) == (void *)0 ||
       (glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)GetGLFunctionAddress("glGenFramebuffers")) == (void *)0 ||
       (glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)GetGLFunctionAddress("glBindFramebuffer")) == (void *)0 ||
       (glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)GetGLFunctionAddress("glFramebufferTexture2D")) == (void *)0 ||
       (glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)GetGLFunctionAddress("glCheckFramebufferStatus")) == (void *)0 ||
       (glBufferSubData = (PFNGLBUFFERSUBDATAPROC)GetGLFunctionAddress("glBufferSubData")) == (void *)0 ||
       (glGetError = (PFNGLGETERRORPROC)GetGLFunctionAddress("glGetError")) == (void *)0 ||
       (glDeleteTextures = (PFNGLDELETETEXTURESPROC)GetGLFunctionAddress("glDeleteTextures")) == (void *)0
       )
    {
        return false;
    }
    return true;
}

#endif
