#ifndef NS_COMMON_H
#define NS_COMMON_H

#if defined(_MSC_VER)
    #define WINDOWS
#else
    #define LINUX
#endif


#if defined(WINDOWS)
    /* Prevents Windows.h from including winsock 1.1 definitions, which conflict with 2.2. */
    #if !defined(WIN32_LEAN_AND_MEAN)
        #define WIN32_LEAN_AND_MEAN
    #endif

    #include <Windows.h>
#elif defined(LINUX)
    #include <pthread.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

typedef uint64_t u64;
typedef int64_t s64;

typedef uint32_t u32;
typedef int32_t s32;

typedef uint16_t u16;
typedef int16_t s16;

typedef uint8_t u8;
typedef int8_t s8;

#define NS_SUCCESS 0
#define NS_ERROR -1
#define NS_MULTIPLE_ERRORS -2
#define NS_TIMED_OUT -3

// TODO: print stack trace instead. use backtrace() and addr2line.
#define DebugPrintInfo() fprintf(stderr, "thread: %lu. %s %s line: %d\n", \
                                 GetThread(), __FILE__, __PRETTY_FUNCTION__, __LINE__)

#if defined(WINDOWS)
    #define GetThread() GetCurrentThreadId()
    #define __PRETTY_FUNCTION__ __FUNCSIG__
#elif defined(LINUX)
    #define GetThread() pthread_self()
    //#define __PRETTY_FUNCTION__ __PRETTY_FUNCTION__
    #define DebugPrintOsInfo() DebugPrintInfo(); perror("    error")
#endif

#define local static
#define global static
#define internal static

#define Kilobytes(NumberOfKbs) (NumberOfKbs * 1024)
#define Megabytes(NumberOfMbs) (NumberOfMbs * 1024 * 1024)
#define Gigabytes(NumberOfGbs) (NumberOfGbs * 1024 * 1024 * 1024)

void ErrorHandler()
{
    /* Just place a breakpoint here. */
    int debug = 0;
}

#define Assert(Expression) if (!(Expression)) ErrorHandler();
#define CrashProgram() *((int *)0) = 0;
#define StallProgram() while (1);

#define GetNumBits(X) (8*sizeof(X))

#define Log(Format, ...) _Log("%s line %d. " Format, __FILE__, __LINE__, __VA_ARGS__)
internal void _Log(const char *Format, ...);

#define _CheckEquals(Actual, Expected, Action, WhatToReturn) \
    { \
        long ErrorActual = (long)(Actual); \
        long ErrorOther = (long)(Expected); \
        if(((ErrorActual) != (ErrorOther))) \
        { \
            Log("Error. Expected Value: 0x%x, Actual Value: 0x%x\n", ErrorOther, ErrorActual); \
            Action; \
            CrashProgram(); \
            WhatToReturn; \
        } \
    }

#define _CheckNotEquals(Actual, Error, Action, WhatToReturn) \
    { \
        long ErrorActual = (long)(Actual); \
        long ErrorOther = (long)(Error); \
        if(((ErrorActual) == (ErrorOther))) \
        { \
            Log("Error. Actual is equal to error: 0x%x\n", ErrorActual); \
            Action; \
            CrashProgram(); \
            WhatToReturn; \
        } \
    }

#define CheckEquals(Actual, Expected) _CheckEquals(Actual, Expected, , )
#define CheckNotEquals(Actual, Expected) _CheckNotEquals(Actual, Expected, , )
#define CheckEquals_R1(Actual, Expected) _CheckEquals(Actual, Expected, , return 1)
#define CheckNotEquals_R1(Actual, Expected) _CheckNotEquals(Actual, Expected, , return 1)
#define CheckEquals_RN(Actual, Expected) _CheckEquals(Actual, Expected, , return NULL)
#define CheckNotEquals_RN(Actual, Expected) _CheckNotEquals(Actual, Expected, , return NULL)
#define CheckEquals_RR(Actual, Expected) _CheckEquals(Actual, Expected, , return Result)
#define CheckNotEquals_RR(Actual, Expected) _CheckNotEquals(Actual, Expected, , return Result)
#define CheckNotEquals_AR(Actual, Expected, Action) _CheckNotEquals(Actual, Expected, Action, )

#define CheckSOK_RR(Actual) CheckEquals_RR(Actual, S_OK)
#define CheckSOK(Actual) CheckEquals(Actual, S_OK)
#define CheckNotZero(Actual) CheckNotEquals(Actual, 0)
#define CheckNotZero_AR(Actual, Action) CheckNotEquals_AR(Actual, 0, Action)
#define CheckNotNull_RR(Actual) CheckNotEquals_RR(Actual, NULL)
#define CheckZero_RR(Actual) CheckEquals_RR(Actual, 0)
#define CheckNotZero_RR(Actual) CheckNotEquals_RR(Actual, 0)
#define CheckNotNeg1_RR(Actual) CheckNotEquals_RR(Actual, -1)
#define CheckNotNeg1(Actual) CheckNotEquals(Actual, -1)

internal void Printf(const char *Format, ...);
internal void Printf(int Value);
internal void Printf(float Value);
internal void Printf(uint32_t Value);

/* Arrays. */

#define ArrayCount(Array) (sizeof(Array)/sizeof(Array[0]))
#define ArrayEnd(Array) (&Array[ArrayCount(Array)])
#define ArrayAdd(Array, ArraySize, NewValue) if (ArraySize < ArrayCount(Array)) { Array[ArraySize++] = NewValue; } else { Assert(false); }
#define ArrayRemove(Array, ArraySize, IndexToRemove) if (ArraySize > 0) { Array[IndexToRemove] = Array[--ArraySize]; } else { Assert(false); } 
#define ArrayPtrAdd(Array, ArrayMaxSize, ArraySize, NewValue) if ((ArraySize) < ArrayMaxSize) { Array[(ArraySize)++] = NewValue; } else { Assert(false); }

template <typename element_type>
bool CheckArrayContains(element_type *Array, int ArrayLength, element_type Value, int *IndexPtr = NULL)
{
    bool Result = false;
    for (int I = 0; I < ArrayLength; I++)
    {
        if (Array[I] == Value)
        {
            Result = true;
            if (IndexPtr)
            {
                *IndexPtr = I;
            }
            break;
        }
    }
    return Result;
}

template <typename element_type>
int GetArrayIndex(element_type *Array, int ArrayLength, element_type Value)
{
    int Result = -1;
    for (int I = 0; I < ArrayLength; I++)
    {
        if (Array[I] == Value)
        {
            Result = I;
            break;
        }
    }
    return Result;
}

template <typename element_type>
void RemoveArrayElement(element_type *Array, int ArrayLength, int RemoveIdx)
{
    for (int I = RemoveIdx; I < ArrayLength - 1; I++)
    {
        Array[I] = Array[I + 1];
    }
}

template <typename element_type>
void InsertArrayElement(element_type *Array, int ArrayLength, int InsertIdx, element_type NewElement)
{
    /* Shift to make room. */
    for (int I = ArrayLength; I > InsertIdx; I--)
    {
        Array[I] = Array[I - 1];
    }

    /* Add new element. */
    Array[InsertIdx] = NewElement;
}

template <typename element_type>
bool CheckForDuplicates(element_type *Array, int ArrayLength)
{
    bool Result = false;
    for (int I = 0; I < ArrayLength; I++)
    {
        for (int J = I + 1; J < ArrayLength; J++)
        {
            Assert(Array[I] != Array[J]);
        }
    }
    return Result;
}

template <typename element_type, typename lambda>
void SortArray(element_type *Array, int ArrayLength, lambda CheckIfLarger)
{
    if (ArrayLength <= 1)
    {
        return;
    }

    for (int SortSize = ArrayLength; SortSize > 0; SortSize--)
    {
        /* Find max. */
        int MaxIdx = 0;
        for (int I = 1; I < SortSize; I++)
        {
            if (CheckIfLarger(Array[I], Array[MaxIdx]))
            {
                MaxIdx = I;
            }
        }

        /* Swap max and end. */
        element_type Tmp = Array[SortSize - 1];
        Array[SortSize - 1] = Array[MaxIdx];
        Array[MaxIdx] = Tmp;
    }
}

template <typename element_type, typename lambda>
int FindSmallest(element_type *Array, int ArrayLength, lambda CheckIfLarger)
{
    int SmallestIdx = 0;
    for (int I = 1; I < ArrayLength; I++)
    {
        if (CheckIfLarger(Array[SmallestIdx], Array[I]))
        {
            SmallestIdx = I;
        }
    }
    int Result = SmallestIdx;
    return Result;
}

template <typename element_type>
void RemoveConsecutiveElements(element_type *Array, int ArrayLength, int RemoveIdx, int NumElementsToRemove)
{
    for (int J = RemoveIdx; J < ArrayLength - NumElementsToRemove; J++)
    {
        Array[J] = Array[J + NumElementsToRemove];
    }
}

int GetRandomNumber(int Min, int MaxExclusive)
{
    int Range = MaxExclusive - Min;
    int RandomNumber = rand() % Range;
    int Result = Min + RandomNumber;
    return Result;
}

#endif
