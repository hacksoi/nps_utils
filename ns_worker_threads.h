#ifndef NS_WORKER_THREADS_H
#define NS_WORKER_THREADS_H

#include "ns_common.h"
#include "ns_thread.h"
#include "ns_math.h"
#include "ns_memory.h"
#include "ns_work_queue.h"


struct NsWorkerThreads
{
    NsThread *threads;
    int thread_capacity;
    NsWorkQueue work_queue;
};


/* Internal */

internal void *
ns_worker_threads_worker_thread_entry(void *thread_input)
{
    int status;
    NsWorkerThreads *worker_threads = (NsWorkerThreads *)thread_input;

    while(1)
    {
        NsWork *work;
        status = ns_work_queue_get(&worker_threads->work_queue, &work);
        if(status != NS_SUCCESS)
        {
            DebugPrintInfo();
            return (void *)status;
        }

        status = (int)work->thread_entry(work->work);
        if(status != NS_SUCCESS)
        {
            return (void *)status;
        }
    }
}

/* API */

int
ns_worker_threads_create(NsWorkerThreads *worker_threads, int max_threads, int max_work)
{
    // we need at least 2... for now
    if(max_work < 2)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    int status;

    status = ns_work_queue_create(&worker_threads->work_queue, max_work);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    NsThread *threads = (NsThread *)ns_memory_allocate(sizeof(NsThread)*max_threads);
    if(threads == NULL)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    for(int i = 0; i < max_threads; i++)
    {
        status = ns_thread_create(&threads[i], ns_worker_threads_worker_thread_entry, worker_threads);
        if(status != NS_SUCCESS)
        {
            DebugPrintInfo();
            return NS_ERROR;
        }
    }

    worker_threads->threads = threads;
    worker_threads->thread_capacity = max_threads;

    return NS_SUCCESS;
}

int
ns_worker_threads_destroy(NsWorkerThreads *worker_threads)
{
    int status;

    ns_memory_free(worker_threads->threads);

    status = ns_work_queue_destroy(&worker_threads->work_queue);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    return NS_SUCCESS;
}

int
ns_worker_threads_add_work(NsWorkerThreads *worker_threads, 
                           void *(*thread_entry)(void *), void *work)
{
    int status;

    status = ns_work_queue_add(&worker_threads->work_queue, thread_entry, work);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    return NS_SUCCESS;
}

#endif
