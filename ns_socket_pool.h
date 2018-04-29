#if 0
TODO: use a free list
#endif

#ifndef NS_SOCKET_POOL_H
#define NS_SOCKET_POOL_H

#include "ns_common.h"
#include "ns_socket.h"
#include "ns_memory.h"
#include "ns_mutex.h"


struct NsSocketPoolSocket
{
    NsSocket socket;
    NsSocketPoolSocket *next;
    NsSocketPoolSocket *prev;
};

struct NsSocketPool
{
    NsMutex mutex;
    NsSocketPoolSocket *sp_sockets;
    NsSocketPoolSocket *free_list_head;

    int offset_from_socket_to_pool_socket;
};


/* Internal */

/* API */

int
ns_socket_pool_create(NsSocketPool *socket_pool, int capacity)
{
    int status;

    NsSocketPoolSocket *sp_sockets = (NsSocketPoolSocket *)ns_memory_allocate(sizeof(NsSocketPoolSocket)*capacity);
    if(sp_sockets == NULL)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    socket_pool->sp_sockets = sp_sockets;
    socket_pool->free_list_head = sp_sockets;

    status = ns_mutex_create(&socket_pool->mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    // setup free list
    for(int i = 0; i < (capacity - 1); i++)
    {
        sp_sockets[i].next = &sp_sockets[i + 1];
    }

    NsSocketPoolSocket *sp_socket = &sp_sockets[0];
    socket_pool->offset_from_socket_to_pool_socket = (int)((uint8_t *)sp_socket - (uint8_t *)&sp_socket->socket);

    return NS_SUCCESS;
}

int
ns_socket_pool_destroy(NsSocketPool *socket_pool)
{
    int status;

    ns_memory_free(socket_pool->sp_sockets);

    status = ns_mutex_destroy(&socket_pool->mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    return NS_SUCCESS;
}

int
ns_socket_pool_get(NsSocketPool *socket_pool, NsSocket **socket_ptr)
{
    int status;

    NsSocketPoolSocket *free = socket_pool->free_list_head;
    if(free == NULL)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    status = ns_mutex_lock(&socket_pool->mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    // remove from free list
    socket_pool->free_list_head = free->next;

    status = ns_mutex_unlock(&socket_pool->mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    *socket_ptr = &free->socket;

    return NS_SUCCESS;
}

int
ns_socket_pool_release(NsSocketPool *socket_pool, NsSocket *socket)
{
    int status;

    status = ns_mutex_lock(&socket_pool->mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    NsSocketPoolSocket *sp_socket = (NsSocketPoolSocket *)((uint8_t *)socket + socket_pool->offset_from_socket_to_pool_socket);

    // add to free list
    sp_socket->next = socket_pool->free_list_head;
    socket_pool->free_list_head = sp_socket;

    status = ns_mutex_unlock(&socket_pool->mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    return NS_SUCCESS;
}

#endif
