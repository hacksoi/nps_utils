#ifndef NS_LOGGING_H
#define NS_LOGGING_H

#include "common.h"

#if defined(WINDOWS)
    #include <Windows.h>
    #define GetThread() GetCurrentThreadId()
#elif defined(LINUX)
    #include <pthread.h>
    #define GetThread() pthread_self()
#else
    #error Windows nor Linux is defined
#endif

#define DebugPrintInfo() fprintf(stdout, "thread: %lu, %s line: %d\n", \
                                 GetThread(), __PRETTY_FUNCTION__, __LINE__)

#endif
