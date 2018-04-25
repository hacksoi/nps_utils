#ifndef NS_THREAD_H
#define NS_THREAD_H

#include "ns_common.h"
#include "ns_mutex.h"

#include <stdlib.h>

#if defined(WINDOWS)
#elif defined(LINUX)
    #include <pthread.h>
#endif


#if defined(WINDOWS)
#elif defined(LINUX)
    typedef pthread_t NsInternalThread;
#endif


/* Internal */

struct NsThread
{
    NsInternalThread internal_thread;

    void *input;
    void *(*entry)(void *);

    void (*completion_callback)(NsThread *);
    void *extra_data_void_ptr;
    int extra_data_int;
};


internal void *
ns_thread_entry(void *input)
{
    NsThread *thread = (NsThread *)input;
    void *result = thread->entry(thread->input);
    if(thread->completion_callback != NULL)
    {
        thread->completion_callback(thread);
        thread->completion_callback = NULL;
    }
    return result;
}


/* API */

int
ns_thread_create(NsThread *thread, void *(*thread_entry)(void *), void *thread_input)
{
    int status;

    thread->entry = thread_entry;
    thread->input = thread_input;

#if defined(WINDOWS)
#elif defined(LINUX)
    status = pthread_create(&thread->internal_thread, NULL, ns_thread_entry, thread);
    if(status != 0)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }
#endif
    return NS_SUCCESS;
}

#endif
