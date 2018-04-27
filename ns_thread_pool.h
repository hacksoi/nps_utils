#if 0
currently, there's still a problem with the fact that the user can call ns_thread_pool_thread_create() on a thread that's already finished executing, therefore causing the pool to not keep track.
#endif

#if 0
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

/* API */

struct NsThreadPoolThread
{
    NsThread thread;
    struct NsThreadPool *thread_pool; // Pointer to thread pool where this thread is from.
};

struct NsThreadPool
{
    NsThreadPoolThread *tp_threads;
    NsMutex mutex;
    void *memory;
    int capacity;
    bool *statuses; // We could place the status in the NsThreadPoolThread, but this is more cache friendly.
};


/* Internal */

internal void
ns_thread_pool_thread_completion_callback(NsThread *thread)
{
    NsThreadPool *thread_pool = (NsThreadPool *)thread->extra_data_void_ptr;
    NsThreadPoolThread *tp_threads = thread_pool->tp_threads;
    bool *statuses = thread_pool->statuses;
    int thread_pool_capacity = thread_pool->capacity;

    int tp_thread_idx = 0;
    for(; tp_thread_idx < thread_pool_capacity; tp_thread_idx++)
    {
        if(thread == &tp_threads[tp_thread_idx].thread)
        {
            break;
        }
    }

    // sanity check
    if(!statuses[tp_thread_idx])
    {
        DebugPrintInfo();
        return;
    }

    statuses[tp_thread_idx] = false;
}

/* API */

int
ns_thread_pool_create(NsThreadPool *thread_pool, int capacity)
{
    int status;

    uint32_t threads_size = (capacity*sizeof(NsThreadPoolThread));
    uint32_t statuses_size = (capacity*sizeof(bool));
    uint8_t *memory = (uint8_t *)malloc(threads_size + statuses_size);
    if(memory == NULL)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    thread_pool->tp_threads = (NsThreadPoolThread *)memory;
    thread_pool->statuses = (bool *)(memory + threads_size);
    thread_pool->capacity = capacity;

    NsThreadPoolThread *tp_threads = thread_pool->tp_threads;
    for(int i = 0; i < capacity; i++)
    {
        tp_threads[i].thread_pool = thread_pool;
    }

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
    free(thread_pool->tp_threads);
    return NS_SUCCESS;
}

int
ns_thread_pool_thread_get(NsThreadPool *thread_pool, NsThreadPoolThread **tp_thread_ptr)
{
    ns_mutex_lock(&thread_pool->mutex);

    NsThreadPoolThread *tp_thread = NULL;
    int thread_pool_capacity = thread_pool->capacity;
    bool *statuses = thread_pool->statuses;
    int tp_thread_idx = 0;
    for(; tp_thread_idx < thread_pool_capacity; tp_thread_idx++)
    {
        if(!statuses[tp_thread_idx])
        {
            tp_thread = &thread_pool->tp_threads[tp_thread_idx];
            statuses[tp_thread_idx] = true;
            break;
        }
    }

    ns_mutex_unlock(&thread_pool->mutex);

    if(tp_thread_idx == thread_pool_capacity)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    *tp_thread_ptr = tp_thread;

    return NS_SUCCESS;
}

int
ns_thread_pool_thread_create(NsThreadPoolThread *tp_thread, 
                             void *(*thread_entry)(void *), void *thread_input)
{
    int status;

    status = ns_thread_create(&tp_thread->thread, thread_entry, thread_input, 
                              ns_thread_pool_thread_completion_callback, tp_thread->thread_pool);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    return NS_SUCCESS;
}

#endif
#endif
