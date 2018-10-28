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

//#define Assert(Expression) assert(Expression);
#define Assert(Expression) if (!(Expression)) (*((int *)0) = 0)
#define CrashProgram() Assert(false)

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
#define CheckEquals_RR(Actual, Expected) _CheckEquals(Actual, Expected, , return {})
#define CheckNotEquals_RR(Actual, Expected) _CheckNotEquals(Actual, Expected, , return {})
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

template <class ArrayPointerType, class Value>
bool CheckArrayContains(ArrayPointerType Array, int ArrayLength, Value Value)
{
    bool Result = false;
    for (int I = 0; I < ArrayLength; I++)
    {
        if (Array[I] == Value)
        {
            Result = true;
            break;
        }
    }
    return Result;
}

#endif
