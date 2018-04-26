#ifndef NS_SOCKET_POOL_H
#define NS_SOCKET_POOL_H

#include "ns_common.h"
#include "ns_socket.h"


struct NsSocketPoolSocket
{
    NsSocket socket;
    struct NsSocketPool *socket_pool; // Points to socket pool where this sp_socket came from.
};

struct NsSocketPool
{
    NsMutex mutex;
    void *memory;
    NsSocketPoolSocket *sp_sockets;
    bool *statuses; // We could place the status in the NsSocketPoolSocket, but this is more cache friendly.
    int capacity;
};


/* Internal */

internal void
ns_socket_pool_socket_close_callback(NsSocket *socket)
{
    NsSocketPool *socket_pool = (NsSocketPool *)socket->extra_data_void_ptr;
    NsSocketPoolSocket *sp_sockets = socket_pool->sp_sockets;
    bool *statuses = socket_pool->statuses;
    int socket_pool_capacity = socket_pool->capacity;

    int sp_socket_idx = 0;
    for(; sp_socket_idx < socket_pool_capacity; sp_socket_idx++)
    {
        if(socket == &sp_sockets[sp_socket_idx].socket)
        {
            break;
        }
    }

    // sanity check
    if(!statuses[sp_socket_idx])
    {
        DebugPrintInfo();
        return;
    }

    statuses[sp_socket_idx] = false;
}

/* API */

int
ns_socket_pool_create(NsSocketPool *socket_pool, int capacity)
{
    int status;

    uint32_t sockets_size = (capacity*sizeof(NsSocketPoolSocket));
    uint32_t statuses_size = (capacity*sizeof(bool));
    uint8_t *memory = (uint8_t *)malloc(sockets_size + statuses_size);
    if(memory == NULL)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    socket_pool->sp_sockets = (NsSocketPoolSocket *)memory;
    socket_pool->statuses = (bool *)(memory + sockets_size);
    socket_pool->capacity = capacity;

    NsSocketPoolSocket *sp_sockets = socket_pool->sp_sockets;
    for(int i = 0; i < capacity; i++)
    {
        sp_sockets[i].socket_pool = socket_pool;
    }

    status = ns_mutex_create(&socket_pool->mutex);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    return NS_SUCCESS;
}

int
ns_socket_pool_destroy(NsSocketPool *socket_pool)
{
    free(socket_pool->sp_sockets);
    return NS_SUCCESS;
}

int
ns_socket_pool_socket_get(NsSocketPool *socket_pool, NsSocketPoolSocket **sp_socket_ptr)
{
    ns_mutex_lock(&socket_pool->mutex);

    NsSocketPoolSocket *sp_socket = NULL;
    int socket_pool_capacity = socket_pool->capacity;
    bool *statuses = socket_pool->statuses;
    int sp_socket_idx = 0;
    for(; sp_socket_idx < socket_pool_capacity; sp_socket_idx++)
    {
        if(!statuses[sp_socket_idx])
        {
            sp_socket = &socket_pool->sp_sockets[sp_socket_idx];
            statuses[sp_socket_idx] = true;
            break;
        }
    }

    ns_mutex_unlock(&socket_pool->mutex);

    if(sp_socket_idx == socket_pool_capacity)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    *sp_socket_ptr = sp_socket;

    return NS_SUCCESS;
}

int
ns_socket_pool_socket_listen(NsSocketPoolSocket *sp_socket, const char *port, int backlog = 10)
{
    int status;

    status = ns_socket_listen(&sp_socket->socket, port, backlog, 
                              ns_socket_pool_socket_close_callback, sp_socket->socket_pool);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    return NS_SUCCESS;
}

#endif
