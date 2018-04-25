#ifndef NS_MUTEX_H
#define NS_MUTEX_H

#include "ns_common.h"

#if defined(WINDOWS)
#else
    #include <pthread.h>
#endif


#if defined(WINDOWS)
#elif defined(LINUX)
    typedef pthread_mutex_t NsInternalMutex;
#endif


/* Internal */

struct NsMutex
{
    NsInternalMutex internal_mutex;
};


/* API */

int
ns_mutex_create(NsMutex *mutex)
{
    int status;

#if defined(WINDOWS)
#elif defined(LINUX)
    status = pthread_mutex_init(&mutex->internal_mutex, NULL);
    if(status != 0)
    {
        DebugPrintInfo();
        return status;
    }
#endif
    return NS_SUCCESS;
}

int 
ns_mutex_lock(NsMutex *mutex)
{
    int status;

#if defined(WINDOWS)
#elif defined(LINUX)
    status = pthread_mutex_lock(&mutex->internal_mutex);
    if(status != 0)
    {
        DebugPrintInfo();
        return status;
    }
#endif
    return NS_SUCCESS;
}

int 
ns_mutex_unlock(NsMutex *mutex)
{
    int status;

#if defined(WINDOWS)
#elif defined(LINUX)
    status = pthread_mutex_unlock(&mutex->internal_mutex);
    if(status != 0)
    {
        DebugPrintInfo();
        return status;
    }
#endif
    return NS_SUCCESS;
}

#endif
