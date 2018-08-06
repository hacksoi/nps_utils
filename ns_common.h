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

#define ArrayCount(Array) (sizeof(Array)/sizeof(Array[0]))

//#define Assert(Expression) if(!(Expression)) *(int *)0 = 0;
#define Assert(Expression) assert(Expression);

#define Log(Format, ...) _Log("%s line %d. " Format, __FILE__, __LINE__, __VA_ARGS__)
internal void _Log(const char *Format, ...);

#define Check(ToCheck) \
    if(!(ToCheck)) \
    { \
        Log("Error\n"); \
        Assert(ToCheck); \
    }

#define CheckR1(ToCheck) \
    if(!(ToCheck)) \
    { \
        Log("Error\n"); \
        Assert(ToCheck); \
        return 1; \
    }

#define CheckRN(ToCheck) \
    if(!(ToCheck)) \
    { \
        Log("Error\n"); \
        Assert(ToCheck); \
        return NULL; \
    }

#define CheckRR(ToCheck) \
    if(!(ToCheck)) \
    { \
        Log("Error\n"); \
        Assert(ToCheck); \
        return Result; \
    }

internal void Printf(const char *Format, ...);
internal void Printf(int Value);
internal void Printf(float Value);

internal void *
Malloc(int Size)
{
    void *Result = malloc(Size);
    return Result;
}

internal void *
MallocZero(int Size)
{
    void *Result = calloc(1, Size);
    return Result;
}

#endif
