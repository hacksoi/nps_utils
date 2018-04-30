#ifndef NS_STRING_H
#define NS_STRING_H

#include "ns_common.h"

#include <string.h>


/* API */

int
ns_string_get_token(char *dst, char *src, int dst_size, char delimiter = ' ')
{
    int len = 0;
    while(src[len] &&
          src[len] != delimiter)
    {
        dst[len] = src[len];
        len++;
    }
    dst[len] = 0;
    return len;
}

/* Checks for equality excluding null-terminators. */
inline bool
ns_string_equals_weak(char *String1, char *String2)
{
    for(; (*String1 && *String2) && (*String1 == *String2); String1++, String2++);
    return !*String2;
}

/* Checks if String1 contains String2. */
inline bool
ns_string_contains(char *String1, char *String2)
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
        if(ns_string_equals_weak(String1, String2))
        {
            return true;
        }
    }

    return false;
}

inline void
ns_string_reverse(char *String, int Length)
{
    for(int i = 0; i < Length/2; i++)
    {
        char Tmp = String[i];
        String[i] = String[Length - i - 1];
        String[Length - i - 1] = Tmp;
    }
}

inline uint32_t
ns_string_from_int(char *Dest, int Value)
{
    if(Value < 0)
    {
        return 0;
    }

    int NumDigits = 0;
    do
    {
        Dest[NumDigits++] = '0' + (Value % 10);
        Value /= 10;
    }
    while(Value > 0);

    ns_string_reverse(Dest, NumDigits);

    return NumDigits;
}

#endif
