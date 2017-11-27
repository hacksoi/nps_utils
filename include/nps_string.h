#ifndef NPS_STRING_H
#define NPS_STRING_H

#include <string.h>

#include "common_defs.h"

/* Checks for equality excluding null-terminators. */
internal inline bool32
StringEqualsWeak(char *String1, char *String2)
{
    for(; (*String1 && *String2) && (*String1 == *String2); String1++, String2++);
    return !*String2;
}

/* Checks if String1 contains String2. */
internal inline bool32
StringContains(char *String1, char *String2)
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
        if(StringEqualsWeak(String1, String2))
        {
            return true;
        }
    }

    return false;
}

#endif
