#ifndef NPS_COMMON_DEFS
#define NPS_COMMON_DEFS

#define local static
#define global static
#define internal static

typedef int bool32;

#define Kilobytes(NumberOfKbs) (NumberOfKbs * 1024)
#define Megabytes(NumberOfMbs) (NumberOfMbs * 1024 * 1024)
#define Gigabytes(NumberOfGbs) (NumberOfGbs * 1024 * 1024 * 1024)

#define ArrayCount(Array) (sizeof(Array)/sizeof(Array[0]))

#endif
