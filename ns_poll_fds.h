#ifndef NS_POLL_FDS_H
#define NS_POLL_FDS_H

#include "ns_poll_fd.h"
#include "ns_socket.h"


#if defined(WINDOWS)
#elif defined(LINUX)
    typedef pollfd NsPollFd;
#endif


struct NsPollFdsWebSocket
{
    // TODO: pollfds should be sorted on the address of the websocket. faster
    // add() and remove() that way

    NsPollFd *pollfds;
    NsCondv empty_condv;
    NsMutex mutex;

    int size;
    int capacity;

    // array to link pollfd idx back to websocket
    NsWebSocket **websockets;
};


/* API */

int
ns_poll_fds_create(NsPollFdsWebSocket *poll_fds, int capacity)
{
    int status;

    poll_fds->pollfds = ns_memory_allocate(sizeof(NsPollFd)*capacity);
    if(poll_fds->poll_fds == NULL)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    poll_fds->websockets = ns_memory_allocate(sizeof(NsWebSocket *)*capacity);
    if(poll_fds->websockets == NULL)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    status = ns_condv_create(&poll_fds->empty_condv);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_mutex_create(&poll_fds->mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    poll_fds->capacity = capacity;
    poll_fds->size = 0;

    return NS_SUCCESS;
}

internal int
ns_poll_fds_add(NsPollFds *poll_fds, NsWebsocket *websocket)
{
    int status;

    if(poll_fds->size >= poll_fds->capacity)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    status = ns_mutex_lock(&poll_fds->mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    NsPollFd *pollfds = poll_fds->pollfds;
    int size = poll_fds->size;

    NsPollFd *pollfd = &pollfds[size];
    pollfd->fd = websocket;
    pollfd->events |= NS_SOCKET_POLL_IN;

    poll_fds->size++;

    status = ns_condv_signal(&poll_fds->empty_condv);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_mutex_unlock(&poll_fds->mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }
}

internal void
ns_poll_fds_remove(NsPollFdsWebSocket *poll_fds, NsWebsocket *websocket)
{
    int status;

    status = ns_mutex_lock(&poll_fds->mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    NsPollFd *pollfds = poll_fds->pollfds;
    int size = poll_fds->size;

    int pollfds_idx = 0;
    for(; pollfds_idx < size; pollfds_idx++)
    {
        if(pollfds[pollfds_idx].websocket == websocket])
        {
            break;
        }
    }

    // sanity check
    if(pollfds_idx == num_websockets)
    {
        DebugPrintInfo();
        return status;
    }

    // shift
    for(int i = pollfds_idx; i < (num_websockets - 1); i++)
    {
        pollfds[i] = pollfds[i + 1];
    }

    poll_fds->size--;

    status = ns_mutex_unlock(&poll_fds->mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }
}

/* Waits until the poll_fds is nonempty and locks the mutex. */
int
ns_poll_fds_wait_till_nonempty_and_lock(NsPollFdsWebSocket *poll_fds)
{
    int status;

    status = ns_mutex_lock(&poll_fds->mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    while(poll_fds->size == 0)
    {
        status = ns_condv_wait(&poll_fds->empty_condv, &poll_fds->mutex);
        if(status != NS_SUCCESS)
        {
            DebugPrintInfo();
            return status;
        }
    }

    return NS_SUCCESS;
}

int
ns_poll_fds_unlock(NsPollFdsWebSocket *poll_fds)
{
    int status;

    status = ns_mutex_unlock(&poll_fds->mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    return NS_SUCCESS;
}

int 
ns_poll_fds_poll(NsPollFdsWebSocket *poll_fds)
{
    ns_socket_poll(pollfds, num_websockets, 10);
}

#endif
