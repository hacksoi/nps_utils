/* NOT JUST FOR STRINGS, ALSO CHARS. */

#ifndef NS_STRING_H
#define NS_STRING_H

#include "ns_common.h"

#include <string.h>

/* Chars */

char ToLowerCase(char C)
{
    char Result = (C >= 'A' && C <= 'Z') ? (C + ('a' - 'A')) : C;
    return Result;
}

bool IsNumber(char C)
{
    bool Result = C >= '0' && C <= '9';
    return Result;
}

inline internal 
uint8_t ConvertHexCharToInt(char HexChar)
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

inline internal 
char ConvertIntToHexChar(uint8_t Int)
{
    Assert(Int < 16);
    char Result;
    if (Int >= 0 && Int <= 9)
    {
        Result = '0' + Int;
    }
    else
    {
        Result = 'a' + (Int - 10);
    }
    return Result;
}

/* Strings */

int
StrLen(char *String)
{
    int Result = (int)strlen(String);
    return Result;
}

int
StrCmp(char *String1, char *String2)
{
    int Result = strcmp(String1, String2);
    return Result;
}

int
StringGetToken(char *dst, char *src, int dst_size, char delimiter = ' ')
{
    int len = 0;
    while(src[len] &&
          src[len] != delimiter &&
          dst_size)
    {
        dst[len] = src[len];
        len++;
    }
    dst[len] = 0;
    return len;
}

/* Checks for equality excluding null-terminators. */
inline bool
CheckStringEqualsWeak(char *String1, char *String2)
{
    for(; (*String1 && *String2) && (*String1 == *String2); String1++, String2++);
    return !*String2;
}

/* Checks if String1 contains String2. */
inline bool
CheckStringContains(char *String1, char *String2)
{
    if(!String1 || !String2)
    {
        return false;
    }

    size_t String2Length = strlen(String2);
    if(String2Length == 0)
    {
        return false;
    }

    char *String1End = String1 + strlen(String1);
    for(; (size_t)(String1End - String1) >= String2Length; String1++)
    {
        if(CheckStringEqualsWeak(String1, String2))
        {
            return true;
        }
    }

    return false;
}

inline void
ReverseString(char *String, int Length)
{
    for(int i = 0; i < Length/2; i++)
    {
        char Tmp = String[i];
        String[i] = String[Length - i - 1];
        String[Length - i - 1] = Tmp;
    }
}

inline uint32_t
ConvertIntToString(char *Dest, int Value)
{
    int NumDigits = 0;
    do
    {
        Dest[NumDigits++] = '0' + (Value % 10);
        Value /= 10;
    }
    while(Value > 0);

    ReverseString(Dest, NumDigits);

    return NumDigits;
}

inline uint32_t
ConvertHexStringToInt(char *HexString, int HexStringLength)
{
    if (HexString[0] == '0' && !IsNumber(HexString[1]))
    {
        Assert(ToLowerCase(HexString[1]) == 'x');
        HexString += 2;
        HexStringLength -= 2;
    }

    uint32_t Result = 0;
    while (HexStringLength--)
    {
        Assert((*HexString >= '0' && *HexString <= '9') || (ToLowerCase(*HexString) >= 'a' && ToLowerCase(*HexString) <= 'f'));
        Result *= 16;
        Result += ConvertHexCharToInt(*HexString);
        HexString++;
    }
    return Result;
}

#endif
