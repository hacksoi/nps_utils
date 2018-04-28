#ifndef NS_CONDV_H
#define NS_CONDV_H

#include "ns_mutex.h"

#if defined(WINDOWS)
#elif defined(LINUX)
    #include <pthread.h>
#endif


#if defined(WINDOWS)
#elif defined(LINUX)
    typedef InternalCondv pthread_cond_t;
#endif

struct NsCondv
{
    InternalCondv internal_condv;
};


/* API */

int
ns_condv_create(NsCondv *condv)
{
    int status;

#if defined(WINDOWS)
#elif defined(LINUX)
    status = pthread_cond_init(&condv->internal_condv, NULL);
    if(status != 0)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }
#endif

    return NS_SUCCESS;
}

int
ns_condv_wait(NsCondv *condv, NsMutex *mutex)
{
    int status;

#if defined(WINDOWS)
#elif defined(LINUX)
    status = pthread_cond_wait(&condv->internal_condv, &mutex->internal_mutex);
    if(status != 0)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }
#endif

    return NS_SUCCESS;
}

int
ns_condv_signal(NsCondv *condv)
{
    int status;

#if defined(WINDOWS)
#elif defined(LINUX)
    status = pthread_cond_signal(&condv->internal_condv);
    if(status != 0)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }
#endif

    return NS_SUCCESS;
}

#endif
