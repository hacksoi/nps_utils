#ifndef NS_TEXTURE_RENDERER_H
#define NS_TEXTURE_RENDERER_H

/* TODO: 
    -filenames are kinda slow due to the compares. 
    */

#include "ns_common_renderer.h"
#include "ns_opengl_functions.h"
#include "ns_common.h"

#define MAX_TEXTURES 128
#define INVALID_TEXTURE_ID 0

/* tr = TextureRenderer */
struct tr_texture
{
    const char *Name;
    v2 Dimensions;
    GLuint Id;
};

struct texture_renderer
{
    common_renderer Common;

    tr_texture Textures[MAX_TEXTURES];
    int NumTextures;

    GLuint LastTextureId;
};

texture_renderer CreateTextureRenderer(int WindowWidth, int WindowHeight, const char *VertexShader, const char *FragmentShader)
{
    texture_renderer Result = {};
    Result.Common = CreateGenericTextureCommonRenderObjects(WindowWidth, WindowHeight, Megabytes(1), VertexShader, FragmentShader);
    Result.NumTextures = 0;
    return Result;
}

texture_renderer CreateTextureRenderer(int WindowWidth, int WindowHeight)
{
    texture_renderer Result = CreateTextureRenderer(WindowWidth, WindowHeight, 
                                                    GlobalTextureRendererVertexShaderSource, GlobalTextureRendererFragmentShaderSource);
    return Result;
}

void AddTexture(texture_renderer *TextureRenderer, const char *Name, unsigned int TextureId, int TextureWidth, int TextureHeight)
{
    Assert(TextureId != 0);

    tr_texture NewTexture;
    NewTexture.Name = Name;
    NewTexture.Dimensions = V2(TextureWidth, TextureHeight);
    NewTexture.Id = TextureId;
    ArrayAdd(TextureRenderer->Textures, TextureRenderer->NumTextures, NewTexture);
}

void AddTexture(texture_renderer *TextureRenderer, const char *FilePath, uint32_t *PixelValuesToMask = NULL, int NumPixelValuesToMask = 0)
{
    int ChannelCount, TextureWidth, TextureHeight;
    stbi_set_flip_vertically_on_load(true);
    uint8_t *TextureData = stbi_load(FilePath, &TextureWidth, &TextureHeight, &ChannelCount, 0);

#if 1
    if (PixelValuesToMask != NULL)
    {
        if (IsLittleEndian())
        {
            /* Reverse the pixel values the user passed. */
            for (int I = 0; I < NumPixelValuesToMask; I++)
            {
                ReverseBytes(&PixelValuesToMask[I]);
            }
        }

        int TotalBytes = 4*TextureWidth*TextureHeight;
        for (int I = 0; I < TotalBytes; I += 4)
        {
            uint32_t *PixelValuePtr = (uint32_t *)&TextureData[I];
            uint32_t PixelValue = *PixelValuePtr;
            bool IsMaskValue = false;
            for (int J = 0; J < NumPixelValuesToMask; J++)
            {
                if (PixelValue == PixelValuesToMask[J])
                {
                    IsMaskValue = true;
                    break;
                }
            }
            if (IsMaskValue)
            {
                /* Remove the alpha component. */
                if (IsLittleEndian())
                {
                    PixelValue &= 0x00ffffff;
                }
                else
                {
                    PixelValue &= 0xffffff00;
                }

                /* Write it back. */
                *PixelValuePtr = PixelValue;
            }
        }
    }
#endif

    unsigned int TextureId;
    glGenTextures(1, &TextureId);
    glBindTexture(GL_TEXTURE_2D, TextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    /* Note: if this crashes, it's probably because the width of the image in bytes is not a multiple of 4 bytes (https://www.khronos.org/opengl/wiki/Common_Mistakes#Texture_upload_and_pixel_reads). */
    glTexImage2D(GL_TEXTURE_2D, 0, ChannelCount == 3 ? GL_RGB : GL_RGBA, TextureWidth, TextureHeight, 0, ChannelCount == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, TextureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(TextureData);

    const char *Basename = GetBasename((char *)FilePath);
    AddTexture(TextureRenderer, (const char *)Basename, TextureId, TextureWidth, TextureHeight);
}


void Render(texture_renderer *TextureRenderer)
{
    BeginRender(&TextureRenderer->Common, TextureRenderer->Common.NumVertexData);
    glBindTexture(GL_TEXTURE_2D, TextureRenderer->LastTextureId);
    Assert(TextureRenderer->Common.NumVertexData % 4 == 0);
    glDrawArrays(GL_TRIANGLES, 0, TextureRenderer->Common.NumVertexData/4);
    EndRender(&TextureRenderer->Common);

    /* Reset. */
    TextureRenderer->Common.NumVertexData = 0;
    TextureRenderer->LastTextureId = INVALID_TEXTURE_ID;
}

tr_texture *GetTexture(texture_renderer *TextureRenderer, const char *Name)
{
    tr_texture *Result = NULL;
    for (int I = 0; I < TextureRenderer->NumTextures; I++)
    {
        tr_texture *Tex = &TextureRenderer->Textures[I];
        if (!strcmp(Tex->Name, Name))
        {
            Result = Tex;
            break;
        }
    }
    Assert(Result != NULL);
    return Result;
}

void DrawTexture(texture_renderer *TextureRenderer, const char *Name, rect2 PosCoords = RECT2(V2_ZERO, V2_ZERO), rect2 TexCoordsPixels = RECT2(V2_ZERO, V2_ZERO),
                 bool DrawReversed = false)
{
    tr_texture *Tex = GetTexture(TextureRenderer, Name);

    if (TextureRenderer->LastTextureId != INVALID_TEXTURE_ID)
    {
        if (TextureRenderer->LastTextureId != Tex->Id)
        {
            Render(TextureRenderer);
        }
    }

    if (PosCoords.Min == V2_ZERO &&
        PosCoords.Max == V2_ZERO)
    {
        /* Do the entire window. */
        PosCoords = RECT2(V2(0.0f, 0.0), V2(TextureRenderer->Common.WindowWidth, TextureRenderer->Common.WindowHeight));
    }

    if (TexCoordsPixels.Min == V2_ZERO && 
        TexCoordsPixels.Max == V2_ZERO)
    {
        /* Do the entire image. */
        TexCoordsPixels = RECT2(V2(0.0f, Tex->Dimensions.Y), V2(Tex->Dimensions.X, 0.0f));
    }

    rect2 TexCoords = TexCoordsPixels;

    /* Fix Y coord. */
    TexCoords.Min.Y = Tex->Dimensions.Y - TexCoordsPixels.Min.Y;
    TexCoords.Max.Y = Tex->Dimensions.Y - TexCoordsPixels.Max.Y;

    /* Normalize. */
    TexCoords /= Tex->Dimensions;
    Assert(TexCoords.Min >= V2_ZERO && TexCoords.Max >= V2_ZERO);

    if (DrawReversed)
    {
        Swap(&TexCoords.Min.X, &TexCoords.Max.X);
    }

    InsertTexture((float *)TextureRenderer->Common.VertexData, TextureRenderer->Common.MaxVertexDataBytes, &TextureRenderer->Common.NumVertexData,
                  QUAD2(PosCoords), QUAD2(TexCoords));

    TextureRenderer->LastTextureId = Tex->Id;
}

void SetCameraPos(texture_renderer *TextureRenderer, v2 NewCameraPos)
{
    TextureRenderer->Common.CameraPos = NewCameraPos;
}

void SetZoom(texture_renderer *TextureRenderer, float NewZoom)
{
    TextureRenderer->Common.Zoom = NewZoom;
}

void SetWindowSize(texture_renderer *TextureRenderer, int NewWindowWidth, int NewWindowHeight)
{
    TextureRenderer->Common.WindowHeight = NewWindowHeight;
    TextureRenderer->Common.WindowWidth = NewWindowWidth;
}

void SetFragShaderDebugValue(texture_renderer *TextureRenderer, int NewDebugValue)
{
    TextureRenderer->Common.FragShaderDebugValue = NewDebugValue;
}

void ResetFragShaderDebugValue(texture_renderer *TextureRenderer)
{
    TextureRenderer->Common.FragShaderDebugValue = 0;
}

#endif