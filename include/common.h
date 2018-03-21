#ifndef NPS_COMMON_DEFS
#define NPS_COMMON_DEFS

#include <stdint.h>

#define local static
#define global static
#define internal static

#define Kilobytes(NumberOfKbs) (NumberOfKbs * 1024)
#define Megabytes(NumberOfMbs) (NumberOfMbs * 1024 * 1024)
#define Gigabytes(NumberOfGbs) (NumberOfGbs * 1024 * 1024 * 1024)

#define ArrayCount(Array) (sizeof(Array)/sizeof(Array[0]))

#define Assert(Expression) if(!(Expression)) *(int *)0 = 0;

#endif
