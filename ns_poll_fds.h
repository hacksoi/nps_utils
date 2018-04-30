#if 0
TODO: make this better
#endif

#ifndef NS_POLL_FDS_H
#define NS_POLL_FDS_H

#include "ns_pollfd.h"
#include "ns_socket.h"
#include "ns_websocket.h"


struct NsWebSocket;
internal NsInternalSocket ns_websocket_get_internal(NsWebSocket *websocket);


enum NsPollFdsContainerType
{
    NS_SOCKET, NS_WEBSOCKET
};

struct NsPollFds
{
    NsPollFd *pollfds;
    NsCondv empty_condv;
    NsMutex mutex;

    int capacity;
    int size;

    // array to link pollfd back to its container (websocket, socket, etc.)
    void **containers;
    NsPollFdsContainerType container_type;
};


/* API */

int
ns_poll_fds_create(NsPollFds *poll_fds, NsPollFdsContainerType container_type, int capacity)
{
    int status;

    NsPollFd *pollfds = (NsPollFd *)ns_memory_allocate(sizeof(NsPollFd)*capacity);
    if(pollfds == NULL)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    poll_fds->containers = (void **)ns_memory_allocate(sizeof(void *)*capacity);
    if(poll_fds->containers == NULL)
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

    for(int i = 0; i < capacity; i++)
    {
        pollfds[i].fd = -1;
    }

    poll_fds->pollfds = pollfds;
    poll_fds->capacity = capacity;
    poll_fds->size = 0;
    poll_fds->container_type = container_type;

    return NS_SUCCESS;
}

int
ns_poll_fds_add(NsPollFds *poll_fds, void *container)
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
    int capacity = poll_fds->capacity;

    int pollfd_idx = 0;
    for(; pollfd_idx < capacity; pollfd_idx++)
    {
        NsPollFd *pfd = &pollfds[pollfd_idx];
        if(pfd->fd < 0)
        {
            break;
        }
    }

    if(pollfd_idx == capacity)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    NsPollFd *pollfd = &pollfds[pollfd_idx];

    switch(poll_fds->container_type)
    {
        case NS_SOCKET:
        {
            pollfd->fd = ns_socket_get_internal((NsSocket *)container);
        } break;

        case NS_WEBSOCKET:
        {
            pollfd->fd = ns_websocket_get_internal((NsWebSocket *)container);
        } break;

        default:
        {
            DebugPrintInfo();
            return NS_ERROR;
        } break;
    }

    pollfd->events = NS_SOCKET_POLL_IN;
    poll_fds->containers[pollfd_idx] = container;
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

    return NS_SUCCESS;
}

int
ns_poll_fds_remove(NsPollFds *poll_fds, void *container)
{
    int status;

    if(poll_fds->size <= 0)
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
    void **containers = poll_fds->containers;
    int capacity = poll_fds->capacity;
    NsPollFd *pollfd = NULL;

    for(int pollfds_idx = 0; pollfds_idx < capacity; pollfds_idx++)
    {
        if(container == containers[pollfds_idx])
        {
            pollfd = &pollfds[pollfds_idx];
            break;
        }
    }

    // sanity check
    if(pollfd == NULL)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    // sanity check
    if(pollfd->fd < 0)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    // set to negative so poll() ignores it.
    pollfd->fd = -1;

    // in case we add at an inopportune time and revent is still set
    pollfd->revents = 0;

    poll_fds->size--;

    status = ns_mutex_unlock(&poll_fds->mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    return NS_SUCCESS;
}

/* Waits until the poll_fds is nonempty and locks the mutex. */
int
ns_poll_fds_wait_till_nonempty_and_lock(NsPollFds *poll_fds)
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
ns_poll_fds_unlock(NsPollFds *poll_fds)
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

NsPollFd *
ns_poll_fds_get(NsPollFds *poll_fds)
{
    return poll_fds->pollfds;
}

int
ns_poll_fds_get_capacity(NsPollFds *poll_fds)
{
    return poll_fds->capacity;
}

void *
ns_poll_fds_get_container(NsPollFds *poll_fds, int idx)
{
    return poll_fds->containers[idx];
}

bool
ns_poll_fds_is_full(NsPollFds *poll_fds)
{
    return poll_fds->size >= poll_fds->capacity;
}

#endif
