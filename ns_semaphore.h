#ifndef NS_SEMAPHORE_H
#define NS_SEMAPHORE_H

#if defined(WINDOWS)
#else
    #include <semaphore.h>
#endif


#if defined(WINDOWS)
    //typedef sem_t InternalSemaphore;
#else
    typedef sem_t InternalSemaphore;
#endif


struct NsSemaphore
{
    InternalSemaphore internal_semaphore;
};


/* API */

int ns_semaphore_create(NsSemaphore *semaphore, int initial_value = 0)
{
#if defined(WINDOWS)
#else
    if(sem_init(&semaphore->internal_semaphore, 0, 0) == -1)
    {
        return NS_ERROR;
    }
#endif
    return NS_SUCCESS;
}

int ns_semaphore_destroy(NsSemaphore *semaphore, int initial_value = 0)
{
#if defined(WINDOWS)
#else
    if(sem_destroy(&semaphore->internal_semaphore) == -1)
    {
        return NS_ERROR;
    }
#endif
    return NS_SUCCESS;
}

int ns_semaphore_close(NsSemaphore *semaphore)
{
#if defined(WINDOWS)
#else
    sem_close(&semaphore->internal_semaphore);
#endif
    return NS_SUCCESS;
}

int ns_semaphore_put(NsSemaphore *semaphore)
{
#if defined(WINDOWS)
#else
    if(sem_post(&semaphore->internal_semaphore) == -1)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }
#endif
    return NS_SUCCESS;
}

int ns_semaphore_get(NsSemaphore *semaphore)
{
#if defined(WINDOWS)
#else
    if(sem_wait(&semaphore->internal_semaphore) == -1)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }
#endif
    return NS_SUCCESS;
}

#endif
