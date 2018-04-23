#ifndef NS_COMMON_H
#define NS_COMMON_H

#if defined(_MSC_VER)
    #define WINDOWS
#else
    #define LINUX
#endif

#include <stdint.h>

/* Preprocessor Directives */
//{

#define local static
#define global static
#define internal static

#define Kilobytes(NumberOfKbs) (NumberOfKbs * 1024)
#define Megabytes(NumberOfMbs) (NumberOfMbs * 1024 * 1024)
#define Gigabytes(NumberOfGbs) (NumberOfGbs * 1024 * 1024 * 1024)

#define ArrayCount(Array) (sizeof(Array)/sizeof(Array[0]))

#define Assert(Expression) if(!(Expression)) *(int *)0 = 0;
//}

#endif
