#ifndef NS_BMF
#define NS_BMF

/* TODO: kerning pairs */

#include "ns_string.h"

#pragma pack(push, 1)

struct bmf_block_header
{
    uint8_t identifier;
    uint32_t size;
};

struct bmf_info_block
{
    bmf_block_header header;
    int16_t fontSize;
    uint8_t bitField;
    uint8_t charSet;
    uint16_t stretchH;
    uint8_t aa;
    uint8_t paddingUp;
    uint8_t paddingRight;
    uint8_t paddingDown;
    uint8_t paddingLeft;
    uint8_t spacingHoriz;
    uint8_t spacingVert;
    uint8_t outline;
    uint8_t firstFontNameChar;
};

struct bmf_common_block
{
    bmf_block_header header;
    uint16_t lineHeight;
    uint16_t base;
    uint16_t scaleW;
    uint16_t scaleH;
    uint16_t pages;
    uint8_t bitField;
    uint8_t alphaChnl;
    uint8_t redChnl;
    uint8_t greenChnl;
    uint8_t blueChnl;
};

struct bmf_pages_block
{
    bmf_block_header header;
    char FirstPageNameChar;
};

struct bmf_char
{
    uint32_t id;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    int16_t xoffset;
    int16_t yoffset;
    int16_t xadvance;
    uint8_t page;
    uint8_t chnl;
};

struct bmf_chars_block
{
    bmf_block_header header;
    bmf_char FirstChar;
};
#pragma pack(pop)

struct ns_bmf
{
    bmf_info_block *InfoBlock;
    bmf_common_block *CommonBlock;
    bmf_pages_block *PagesBlock;
    bmf_chars_block *CharsBlock;

    bmf_char *Chars;

    bmf_char *
    GetChar(int Idx)
    {
        bmf_char *Char = this->Chars;
        while (Idx--)
        {
            Char++;
        }
        return Char;
    }
};

internal bmf_block_header *
GetNextBmfBlock(bmf_block_header *Header)
{
    bmf_block_header *Result = (bmf_block_header *)((uint8_t *)(Header + 1) + Header->size);
    return Result;
}

internal ns_bmf
LoadBmf(const char *Filename)
{
    ns_file File = LoadFile(Filename);
    Assert(!MemCmp((char *)File.Contents, (char *)"BMF", 3));
    Assert(File.Contents[3] == 3);

    ns_bmf Result;
    Result.InfoBlock = (bmf_info_block *)(File.Contents + 4);
    Result.CommonBlock = (bmf_common_block *)GetNextBmfBlock(&Result.InfoBlock->header);
    Result.PagesBlock = (bmf_pages_block *)GetNextBmfBlock(&Result.CommonBlock->header);
    Result.CharsBlock = (bmf_chars_block *)GetNextBmfBlock(&Result.PagesBlock->header);
    Result.Chars = (bmf_char *)&Result.CharsBlock->FirstChar;

    return Result;
}

#endif