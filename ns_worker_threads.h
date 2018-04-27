#ifndef NS_WORKER_THREADS_H
#define NS_WORKER_THREADS_H

#include "ns_common.h"
#include "ns_thread.h"
#include "ns_math.h"

#include <stdlib.h>


struct NsWorkerThreads
{
    NsThread *threads;
    int thread_capacity;
    NsWorkerThreadWorkQueue work_queue;
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

        status = work->thread_entry(work->work);
        if(status != NS_SUCCESS)
        {
            DebugPrintInfo();
            return (void *)status;
        }
    }
}

/* API */

int
ns_worker_threads_create(NsWorkerThreads *worker_threads, int max_threads, int max_work)
{
    int status;

    NsThreads *threads = (NsThread *)malloc(sizeof(NsThread)*max_threads);
    if(threads == NULL)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    for(int i = 0; i < max_threads; i++)
    {
        status = ns_thread_create(&threads[i], ns_worker_threads_worker_thread_entry, worker_threads);
        if(status != NS_SUCCCESS)
        {
            DebugPrintInfo();
            return NS_ERROR;
        }
    }

    worker_threads->threads;
    worker_threads->thread_capacity = max_threads;

    return NS_SUCCESS;
}

int
ns_worker_threads_destroy(NsWorkerThreads *worker_threads)
{
    int status;

    free(worker_threads->threads);

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
                           int (*thread_entry)(void *), void *work)
{
    status = ns_work_queue_add(&worker_threads->work_queue, thread_entry, work);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }
    return NS_SUCCESS;
}

#endif
