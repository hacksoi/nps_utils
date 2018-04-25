#ifndef NS_THREAD_POOL_H
#define NS_THREAD_POOL_H

#include "ns_common.h"
#include "ns_thread.h"
#include "ns_mutex.h"

#include <stdlib.h>

#if defined(WINDOWS)
#elif defined(LINUX)
    #include <pthread.h>
#endif


/* Internal */

struct NsThreadPool
{
#if defined(WINDOWS)
#elif defined(LINUX)
    NsThread *threads;
    NsMutex mutex;
#endif
    void *memory;
    int capacity;
    bool *statuses;
};


internal void
ns_thread_pool_thread_completion_callback(NsThread *thread)
{
    NsThreadPool *thread_pool = (NsThreadPool *)thread->extra_data_void_ptr;
    int thread_idx = thread->extra_data_int;

    // sanity check
    if(!thread_pool->statuses[thread_idx])
    {
        DebugPrintInfo();
    }

    thread_pool->statuses[thread_idx] = false;
}

/* API */

int
ns_thread_pool_create(NsThreadPool *thread_pool, int capacity)
{
    int status;

    uint32_t threads_size = (capacity*sizeof(NsThread));
    uint32_t statuses_size = (capacity*sizeof(bool));
    uint8_t *memory = (uint8_t *)malloc(threads_size + statuses_size);
    if(memory == NULL)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    thread_pool->threads = (NsThread *)memory;
    thread_pool->statuses = (bool *)(memory + threads_size);
    thread_pool->capacity = capacity;

    status = ns_mutex_create(&thread_pool->mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    return NS_SUCCESS;
}

int
ns_thread_pool_destroy(NsThreadPool *thread_pool)
{
    free(thread_pool->threads);
    return NS_SUCCESS;
}

int
ns_thread_pool_get(NsThreadPool *thread_pool, NsThread **thread_ptr)
{
    ns_mutex_lock(&thread_pool->mutex);

    NsThread *thread;
    bool *statuses = thread_pool->statuses;
    int thread_pool_capacity = thread_pool->capacity;
    int thread_idx = 0;
    for(; thread_idx < thread_pool_capacity; thread_idx++)
    {
        if(!statuses[thread_idx])
        {
            thread = &thread_pool->threads[thread_idx];
            statuses[thread_idx] = true;
            break;
        }
    }

    ns_mutex_unlock(&thread_pool->mutex);

    if(thread_idx == thread_pool_capacity)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    thread->extra_data_void_ptr = thread_pool;
    thread->extra_data_int = thread_idx;
    thread->completion_callback = ns_thread_pool_thread_completion_callback;
    *thread_ptr = thread;

    return NS_SUCCESS;
}

int
ns_thread_pool_create_thread(NsThreadPool *thread_pool, void *(*thread_entry)(void *), void *thread_input)
{
    int status;

    NsThread *thread;
    status = ns_thread_pool_get(thread_pool, &thread);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    status = ns_thread_create(thread, thread_entry, thread_input);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    return NS_SUCCESS;
}

#endif
