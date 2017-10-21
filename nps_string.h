#ifndef NPS_STRING_H
#define NPS_STRING_H

#include <string.h>

/* Checks for equality excluding null-terminators. */
internal inline bool32
npsStringEqualsWeak(char *String1, char *String2)
{
    for(; (*String1 && *String2) && (*String1 == *String2); String1++, String2++);
    return !*String2;
}

/* Checks if String1 contains String2. */
internal inline bool32
npsStringContains(char *String1, char *String2)
{
    char *String1End = String1 + strlen(String1);
    size_t String2Length = strlen(String2);

    for(; (size_t)(String1End - String1) >= String2Length; String1++)
    {
        if(npsStringEqualsWeak(String1, String2))
        {
            return true;
        }
    }

    return false;
}

#endif
