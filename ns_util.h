#ifndef NS_UTIL_H
#define NS_UTIL_H

#include <stdint.h>
#include "ns_string.h"


/* Bits */

uint32_t ns_left_rotate(uint32_t value, int rots)
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

uint16_t ns_get16be(uint8_t *src)
{
    uint16_t result = ((src[0] << 8) |
                       (src[1] << 0));
    return result;
}

uint64_t ns_get64be(uint8_t *src)
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

void ns_put16be(uint8_t *dest, uint16_t src)
{
    dest[0] = ((src & 0xff00) >> 8);
    dest[1] = ((src & 0x00ff) >> 0);
}

void ns_put64be(uint8_t *dest, uint64_t src)
{
    dest[0] = (uint8_t)((src & 0xff00000000000000) >> 56);
    dest[1] = (uint8_t)((src & 0x00ff000000000000) >> 48);
    dest[2] = (uint8_t)((src & 0x0000ff0000000000) >> 40);
    dest[3] = (uint8_t)((src & 0x000000ff00000000) >> 32);
    dest[4] = (uint8_t)((src & 0x00000000ff000000) >> 24);
    dest[5] = (uint8_t)((src & 0x0000000000ff0000) >> 16);
    dest[6] = (uint8_t)((src & 0x000000000000ff00) >>  8);
    dest[7] = (uint8_t)((src & 0x00000000000000ff) >>  0);
}

/* Base64 */

inline char ns_to_base64(int Value)
{
    int Result;
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
    return (char)Result;
}

/* i.e. "FFFFFF" to (1, 1, 1, 1) */
inline internal v4 ns_hex_string_to_vec(const char *HexString)
{
    Assert(strlen(HexString) == 6);

    v4 Result;
    for(int i = 0; i < 3; i++)
    {
        char *ColorValue = (char *)&HexString[2*i];
        Result[i] = (ConvertHexStringToInt(ColorValue, 2) / 255.0f);
    }
    Result[3] = 1.0f;
    return Result;
}

#endif
