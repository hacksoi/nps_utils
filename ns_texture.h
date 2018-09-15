#ifndef NS_TEXTURE_H
#define NS_TEXTURE_H

#include "ns_common.h"

struct ns_texture
{
    int Width;
    int Height;
    uint8_t *Data;
};

internal void
Free(ns_texture *TextureData)
{
    MemFree(TextureData->Data);
}

#endif