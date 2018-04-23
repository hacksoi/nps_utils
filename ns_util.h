#ifndef NS_UTIL_H
#define NS_UTIL_H

#include <stdint.h>


/* Bits */

uint32_t left_rotate(uint32_t value, int rots)
{
    uint32_t mask = 0x80000000;
    for(int i = 1; i < rots; i++)
    {
        mask = ((mask >> 1) | 0x80000000);
    }

    uint32_t shifted_off_bits = ((value & mask) >> (32 - rots));
    uint32_t rotated_value = ((value << rots) | shifted_off_bits);
    return rotated_value;
}

/* Bytes */

uint16_t get16be(uint8_t *src)
{
    uint16_t result = ((src[0] << 8) |
                       (src[1] << 0));
    return result;
}

uint64_t get64be(uint8_t *src)
{
    uint16_t result = (((uint64_t)src[0] << 56) |
                       ((uint64_t)src[1] << 48) |
                       ((uint64_t)src[2] << 40) |
                       ((uint64_t)src[3] << 32) |
                       ((uint64_t)src[4] << 24) |
                       ((uint64_t)src[5] << 16) |
                       ((uint64_t)src[6] << 8 ) |
                       ((uint64_t)src[7] << 0 ));
    return result;
}

void put16be(uint8_t *dest, uint16_t src)
{
    dest[0] = ((src & 0xff00) >> 8);
    dest[1] = ((src & 0x00ff) >> 0);
}

void put64be(uint8_t *dest, uint64_t src)
{
    dest[0] = ((src & 0xff00000000000000) >> 56);
    dest[1] = ((src & 0x00ff000000000000) >> 48);
    dest[2] = ((src & 0x0000ff0000000000) >> 40);
    dest[3] = ((src & 0x000000ff00000000) >> 32);
    dest[4] = ((src & 0x00000000ff000000) >> 24);
    dest[5] = ((src & 0x0000000000ff0000) >> 16);
    dest[6] = ((src & 0x000000000000ff00) >>  8);
    dest[7] = ((src & 0x00000000000000ff) >>  0);
}

/* Hex */

inline internal int
hex_to_int(char HexChar)
{
    int Result = -1;
    if(HexChar >= '0' && HexChar <= '9')
    {
        Result = HexChar - '0';
    }
    else if(HexChar >= 'A' && HexChar <= 'F')
    {
        Result = HexChar - 'A' + 10;
    }
    else if(HexChar >= 'a' && HexChar <= 'f')
    {
        Result = HexChar - 'a' + 10;
    }
    return Result;
}

inline internal int
hex_to_int(char *HexString, uint64_t Length = 0)
{
    if(Length == 0)
    {
        Length = strlen(HexString);
    }

    int Result = 0;
    for(int i = 0; i < Length; i++)
    {
        Result *= 16;
        Result += HexToInt(HexString[i]);
    }
    return Result;
}

/* i.e. "FFFFFF" to (1, 1, 1, 1) */
inline internal v4
from_hex_color(char *HexString)
{
    Assert(strlen(HexString) == 6);

    v4 Result;
    for(int i = 0; i < 3; i++)
    {
        char *ColorValue = &HexString[2*i];
        Result[i] = (HexToInt(ColorValue, 2) / 255.0f);
    }
    Result[3] = 1.0f;
    return Result;
}

/* Base64 */

inline char int_to_base64(int Value)
{
    char Result;
    if(Value <= 25)
    {
        Result = 'A' + Value;
    }
    else if(Value <= 51)
    {
        Result = 'a' + (Value - 26);
    }
    else if(Value <= 61)
    {
        Result = '0' + (Value - 52);
    }
    else if(Value == 62)
    {
        Result = '+';
    }
    else if(Value == 63)
    {
        Result = '/';
    }
    else
    {
        Result = -1;
    }
    return Result;
}

#endif