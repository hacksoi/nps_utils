#ifndef NS_TGA
#define NS_TGA

#pragma pack(push, 1)
struct tga_header
{
   int8_t IdLength;
   int8_t ColourMapType;
   int8_t DataTypeCode;
   int16_t ColourMapOrigin;
   int16_t ColourMapLength;
   int8_t  ColourMapDepth;
   int16_t X_Origin;
   int16_t Y_Origin;
   int16_t Width;
   int16_t Height;
   int8_t BitsPerPixel;
   int8_t ImageDescriptor;
};
#pragma pack(pop)

struct ns_tga
{
    ns_file File;
    tga_header *Header;
    uint8_t *Data;
};

ns_tga
LoadTga(const char *Name)
{
    ns_file File = LoadFile(Name);
    ns_tga Result;
    Result.File = File;
    Result.Header = (tga_header *)File.Contents;
    Result.Data = (uint8_t *)(Result.Header + 1);
    return Result;
}

internal void
Free(ns_tga Tga)
{
    Free(&Tga.File);
}

#endif