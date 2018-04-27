// TODO: rename to ns_circular_queue_work.h?

#ifndef NS_WORKER_THREADS_H
#define NS_WORKER_THREADS_H

#include "ns_mutex.h"
#include "ns_semaphore.h"


struct NsWork
{
    int (*thread_entry)(void *);
    void *work;
};

struct NsWorkQueue
{
    NsWork *start;
    NsWork *end;
    NsWork *head;
    NsWork *tail;

    NsMutex add_mutex;
    NsMutex get_mutex;
    NsSemaphore semaphore;
};


/* Internal */

internal NsWork *
ns_work_queue_get_next(NsWorkQueue *work_queue, NsWork *work)
{
    // sanity check
    if(work < work_queue->start || 
       work >= work_queue->end)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    NsWork *next = work + 1;
    if(next == work_queue->end)
    {
        next = work_queue->start;
    }
    return next;
}

/* API */

int
ns_work_queue_create(NsWorkQueue *work_queue, int max_work)
{
    int status;

    status = ns_semaphore_create(&worker_queue->semaphore);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_mutex_create(&worker_queue->add_mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_mutex_create(&worker_queue->get_mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    NsWork *work = (NsWork *)malloc(sizeof(NsWork)*max_work);
    if(work == NULL)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    work_queue->start = work;
    work_queue->end = work + max_work;
    work_queue->head = work_queue->start;
    work_queue->tail = work_queue->start;

    return NS_SUCCESS;
}

int
ns_work_queue_destroy(NsWorkQueue *work_queue)
{
    int status;

    free(work_queue->start);

    status = ns_semaphore_destroy(&work_queue->semaphore);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_mutex_destroy(&work_queue->add_semaphore);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_mutex_destroy(&work_queue->get_semaphore);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    return NS_SUCCESS;
}

int 
ns_work_queue_get(NsWorkQueue *work_queue, NsWork **OUT_work_ptr)
{
    int status;

    status = ns_semaphore_get(work_queue->semaphore);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_mutex_lock(work_queue->get_mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    NsWork *head = work_queue->head;
    *OUT_work_ptr = head;
    work_queue->head = ns_work_queue_get_next(work_queue, head);

    return NS_SUCCESS;
}

int 
ns_work_queue_add(NsWorkQueue *work_queue, 
                  int (*worker_thread_entry)(void *), void *work)
{
    int status;

    status = ns_mutex_lock(work_queue->add_mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    NsWork *tail = work_queue->tail;
    NsWork *next_tail = ns_work_queue_get_next(tail);

    // is there enough room?
    if(next_tail == work_queue->head)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    tail->thread_entry = worker_thread_entry;
    tail->work = work;

    work_queue->tail = next_tail;

    status = ns_mutex_unlock(work_queue->add_mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_semaphore_put(work_queue->semaphore);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    return NS_SUCCESS;
}

#endif
