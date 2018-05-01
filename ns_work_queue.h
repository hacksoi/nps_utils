// TODO: rename to ns_circular_queue_work.h?

#ifndef NS_WORK_QUEUE_H
#define NS_WORK_QUEUE_H

#include "ns_mutex.h"
#include "ns_semaphore.h"
#include "ns_memory.h"


struct NsWork
{
    void *(*thread_entry)(void *);
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
        return NULL;
    }

    NsWork *next = work + 1;
    if(next == work_queue->end)
    {
        next = work_queue->start;
    }
    return next;
}

internal void
ns_work_queue_print(NsWorkQueue *work_queue)
{
    printf("start: %p, end: %p, head: %p, tail: %p\n", 
           work_queue->start, work_queue->end, work_queue->head, work_queue->tail);
}

/* API */

int
ns_work_queue_create(NsWorkQueue *work_queue, int max_work)
{
    int status;

    status = ns_semaphore_create(&work_queue->semaphore);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_mutex_create(&work_queue->add_mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_mutex_create(&work_queue->get_mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    // technically, one element is unused
    max_work++;

    NsWork *work = (NsWork *)ns_memory_allocate(sizeof(NsWork)*max_work);
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

    ns_memory_free(work_queue->start);

    status = ns_semaphore_destroy(&work_queue->semaphore);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_mutex_destroy(&work_queue->add_mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_mutex_destroy(&work_queue->get_mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    return NS_SUCCESS;
}

int 
ns_work_queue_get(NsWorkQueue *work_queue, NsWork **work_ptr)
{
    int status;

    status = ns_semaphore_get(&work_queue->semaphore);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_mutex_lock(&work_queue->get_mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    NsWork *head = work_queue->head;
    *work_ptr = head;
    work_queue->head = ns_work_queue_get_next(work_queue, head);

    status = ns_mutex_unlock(&work_queue->get_mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    return NS_SUCCESS;
}

int 
ns_work_queue_add(NsWorkQueue *work_queue, 
                  void *(*worker_thread_entry)(void *), void *work)
{
    int status;

    status = ns_mutex_lock(&work_queue->add_mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    NsWork *tail = work_queue->tail;
    NsWork *next_tail = ns_work_queue_get_next(work_queue, tail);

    // is there enough room?
    if(next_tail == work_queue->head)
    {
        ns_work_queue_print(work_queue);
        DebugPrintInfo();
        return NS_ERROR;
    }

    tail->thread_entry = worker_thread_entry;
    tail->work = work;

    work_queue->tail = next_tail;

    status = ns_mutex_unlock(&work_queue->add_mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_semaphore_put(&work_queue->semaphore);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    return NS_SUCCESS;
}

#endif
