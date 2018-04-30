#if 0
TODO: add some sort of error callback
#endif

#ifndef NS_WEBSOCKET_H
#define NS_WEBSOCKET_H

#include "ns_common.h"
#include "ns_util.h"
#include "ns_sha1.h"
#include "ns_message_queue.h"
#include "ns_socket.h"
#include "ns_thread_pool.h"
#include "ns_condv.h"
#include "ns_memory.h"
#include "ns_worker_threads.h"
#include "ns_poll_fds.h"


#ifndef NS_WEBSOCKET_MAX_CONNECTIONS
    #define NS_WEBSOCKET_MAX_CONNECTIONS 1024
#endif

#define NS_WEBSOCKET_KEY_HEADER "Sec-WebSocket-Key: "
#define NS_WEBSOCKET_KEY_HEADER_LENGTH strlen(NS_WEBSOCKET_KEY_HEADER)

#define NS_WEBSOCKET_OPCODE_CONTINUATION 0x00
#define NS_WEBSOCKET_OPCODE_TEXT 0x01
#define NS_WEBSOCKET_OPCODE_BINARY 0x02
#define NS_WEBSOCKET_OPCODE_CONNECTION_CLOSE 0x08
#define NS_WEBSOCKET_OPCODE_PING 0x09
#define NS_WEBSOCKET_OPCODE_PONG 0x0A


struct NsWebSocketMessage
{
    // we shove these extra things in here so that it's a littler easier to pass
    // from the receiver to the handler thread
    NsWebSocket *websocket; // websocket this message belongs to
    uint8_t *raw_frame;
    int raw_frame_length;

    uint8_t *payload;
    int payload_length;

    NsWebSocketMessage *next;
};

struct NsWebSocket
{
    NsSocket socket;

    NsSemaphore message_semaphore;
    NsWebSocketMessage *message_head;
    NsWebSocketMessage *message_tail;
};

struct NsWebSocketFrame
{
    bool fin;
    bool rsv1;
    bool rsv2;
    bool rsv3;
    uint32_t opcode;
    bool mask;
    uint64_t payload_length;
    uint8_t *mask_key; // 4 consecutive bytes
    uint8_t *payload;
};

struct NsWebSocketContext
{
    NsWorkerThreads worker_threads;
    NsPollFds poll_fds;
    NsWebSocket *websockets[NS_WEBSOCKET_MAX_CONNECTIONS];
};


global NsWebSocketContext ns_websocket_context;


int ns_websocket_create(NsWebSocket *websocket, const char *port);
int ns_websocket_destroy(NsWebSocket *websocket);
int ns_websocket_get_peer(NsWebSocket *websocket, NsWebSocket *peer_websocket);
int ns_websocket_receive(NsWebSocket *websocket, uint8_t *dest, uint32_t dest_size);
int ns_websocket_send(NsWebSocket *websocket, uint8_t *message, uint32_t message_length);
int ns_websocket_close(NsWebSocket *websocket);


/* Internal */

internal void 
ns_websocket_frame_print(NsWebSocketFrame frame)
{
    printf("fin: 0x%x, rsv1: 0x%x, rsv2: 0x%x, rsv3: 0x%x, opcode: 0x%x, mask: 0x%x, "
           "payload_length: %llu, mask[0]: 0x%x, mask[1]: 0x%x, mask[2]: 0x%x, mask[3]: 0x%x\n", 
           frame.fin, frame.rsv1, frame.rsv2, frame.rsv3, frame.opcode, frame.mask, 
           frame.payload_length, frame.mask_key[0], frame.mask_key[1], frame.mask_key[2], frame.mask_key[3]);
}

internal NsWebSocketFrame 
ns_websocket_frame_inflate(uint8_t *raw_frame)
{
    NsWebSocketFrame frame;
    frame.fin = (raw_frame[0] & 0x80);
    frame.rsv1 = (raw_frame[0] & 0x40);
    frame.rsv2 = (raw_frame[0] & 0x20);
    frame.rsv3 = (raw_frame[0] & 0x10);
    frame.opcode = (raw_frame[0] & 0x0f);
    frame.mask = (raw_frame[1] & 0x80);

    // calculate payload length
    frame.payload_length = (raw_frame[1] & 0x7f);
    if(frame.payload_length <= 125)
    {
        frame.mask_key = &raw_frame[2];
    }
    else if(frame.payload_length == 126)
    {
        frame.payload_length = ns_get16be(&raw_frame[2]);
        frame.mask_key = &raw_frame[4];
    }
    else if(frame.payload_length == 127)
    {
        frame.payload_length = ns_get64be(&raw_frame[2]);
        frame.mask_key = &raw_frame[10];
    }

    frame.payload = (frame.mask_key + 4);

    return frame;
}

/* message linked list */
//{
internal int
ns_websocket_message_add(NsWebSocket *websocket, NsWebSocketMessage *message)
{
    int status;

    message->next = NULL;

    // empty?
    if(websocket->message_head == NULL)
    {
        websocket->message_head = message;
        websocket->message_tail = message;
    }
    else
    {
        websocket->message_tail->next = message;
        websocket->message_tail = message;
    }

    status = ns_semaphore_put(&websocket->message_semaphore);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    return NS_SUCCESS;
}

internal int
ns_websocket_message_get(NsWebSocket *websocket, uint8_t *dest, uint32_t dest_size)
{
    int status;

    status = ns_semaphore_get(&websocket->message_semaphore);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    NsWebSocketMessage *head = websocket->message_head;

    // sanity check
    if(head == NULL)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    int bytes_to_copy = min((uint32_t)head->payload_length, (uint32_t)dest_size);
    memcpy(dest, head->payload, bytes_to_copy);

    // did we copy the whole message?
    if(bytes_to_copy == head->payload_length)
    {
        // single element?  
        if(head->next == NULL)
        {
            websocket->message_head = NULL;
            websocket->message_tail = NULL;
        }
        else
        {
            websocket->message_head = head->next;
        }
        ns_memory_free(head);
    }
    else
    {
        head->payload += bytes_to_copy;
        head->payload_length -= bytes_to_copy;

        // we didn't read the whole message, so reverse our decrement
        status = ns_semaphore_put(&websocket->message_semaphore);
        if(status != NS_SUCCESS)
        {
            DebugPrintInfo();
            return status;
        }
    }

    return bytes_to_copy;
}
//}

internal void *
ns_websocket_message_handler_thread_entry(void *thread_input)
{
    int status;

    NsWebSocketMessage *new_message = (NsWebSocketMessage *)thread_input;
    NsWebSocket *websocket = new_message->websocket;
    NsSocket *socket = &websocket->socket;
    uint8_t *raw_frame = new_message->raw_frame;
    int raw_frame_length = new_message->raw_frame_length;

    NsWebSocketFrame frame = ns_websocket_frame_inflate(raw_frame);
    switch(frame.opcode)
    {
        case NS_WEBSOCKET_OPCODE_CONTINUATION:
        case NS_WEBSOCKET_OPCODE_TEXT:
        case NS_WEBSOCKET_OPCODE_BINARY:
        {
            // decode
            for(uint32_t i = 0; i < frame.payload_length; i++)
            {
                frame.payload[i] ^= frame.mask_key[i % 4];
            }

            new_message->payload = frame.payload;
            new_message->payload_length = frame.payload_length;
            ns_websocket_message_add(websocket, new_message);
        } break;

        case NS_WEBSOCKET_OPCODE_CONNECTION_CLOSE:
        {
            printf("websocket: received close request. closing...\n");

            // finish closing process
            int bytes_sent = ns_socket_send(socket, raw_frame, raw_frame_length);
            if(bytes_sent < 0)
            {
                DebugPrintInfo();
                return (void *)bytes_sent;
            }

            // user should close websocket
        } break;

        case NS_WEBSOCKET_OPCODE_PING:
        {
            printf("received a ping\n");

            // change opcode to pong
            raw_frame[0] &= ~NS_WEBSOCKET_OPCODE_PING;
            raw_frame[0] |= NS_WEBSOCKET_OPCODE_PONG;

            // send pong
            int bytes_sent = ns_socket_send(socket, raw_frame, raw_frame_length);
            if(bytes_sent != raw_frame_length)
            {
                DebugPrintInfo();
                return (void *)bytes_sent;
            }

            ns_memory_free(new_message);
        } break;

        default:
        {
            DebugPrintInfo();
        } break;
    }

    return (void *)NS_SUCCESS;
}

internal void *
ns_websocket_receiver_thread_entry(void *thread_data)
{
    int status;
    NsPollFd *pollfds = ns_poll_fds_get(&ns_websocket_context.poll_fds);
    int pollfds_capacity = ns_poll_fds_get_capacity(&ns_websocket_context.poll_fds);

    while(1)
    {
        status = ns_poll_fds_wait_till_nonempty_and_lock(&ns_websocket_context.poll_fds);
        if(status != NS_SUCCESS)
        {
            DebugPrintInfo();
            return (void *)status;
        }

        int num_fds_ready = ns_socket_poll(pollfds, pollfds_capacity, 10);
        if(num_fds_ready < 0)
        {
            DebugPrintInfo();
            return (void *)NS_ERROR;
        }

        status = ns_poll_fds_unlock(&ns_websocket_context.poll_fds);
        if(status != NS_SUCCESS)
        {
            DebugPrintInfo();
            return (void *)status;
        }

        if(num_fds_ready > 0)
        {
            for(int i = 0; i < pollfds_capacity; i++)
            {
                NsPollFd *pollfd = &pollfds[i];
                if(pollfd->fd >= 0)
                {
                    // is this websocket ready for reading?
                    if((pollfd->revents & NS_SOCKET_POLL_IN) != 0)
                    {
                        NsWebSocket *websocket = (NsWebSocket *)ns_poll_fds_get_container(&ns_websocket_context.poll_fds, i);
                        NsSocket *socket = &websocket->socket;

                        int message_size = ns_socket_get_bytes_available(socket);
                        if(message_size > 0)
                        {
                            NsWebSocketMessage *message = (NsWebSocketMessage *)ns_memory_allocate(sizeof(NsWebSocketMessage) + message_size);
                            uint8_t *raw_frame = (uint8_t *)(message + 1);

                            int bytes_received = ns_socket_receive(socket, raw_frame, message_size);
                            if(bytes_received == message_size)
                            {
                                // sanity check
                                if(bytes_received != message_size)
                                {
                                    DebugPrintInfo();
                                    return (void *)NS_ERROR;
                                }

                                message->websocket = websocket;
                                message->raw_frame = raw_frame;
                                message->raw_frame_length = bytes_received;

                                status = ns_worker_threads_add_work(&ns_websocket_context.worker_threads, 
                                                                    ns_websocket_message_handler_thread_entry, (void *)message);
                                if(status != NS_SUCCESS)
                                {
                                    DebugPrintInfo();
                                    return (void *)status;
                                }
                            }
                            else if(bytes_received == 0)
                            {
                                // user should close websocket
                            }
                            else
                            {
                                DebugPrintInfo();
                                return (void *)bytes_received;
                            }
                        }
                        else if(message_size == 0)
                        {
                            // user should close websocket
                        }
                        // were we removed?
                        else if(message_size == NS_SOCKET_BAD_FD &&
                                pollfd->fd == -1)
                        {
                            printf("websocket: socket removed right out from under our noses!\n");
                        }
                        else
                        {
                            DebugPrintInfo();
                            return (void *)message_size;
                        }
                    }
                }
            }
        }
    }

    return (void *)NS_SUCCESS;
}

internal NsInternalSocket
ns_websocket_get_internal(NsWebSocket *websocket)
{
    NsInternalSocket internal_socket = ns_socket_get_internal(&websocket->socket);
    return internal_socket;
}

/* API */

int
ns_websockets_startup(int max_connections, int max_threads)
{
    int status;

    // we need at least 2 threads: receiving requests and processing requests
    if(max_threads < 2)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    status = ns_sockets_startup();
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_worker_threads_create(&ns_websocket_context.worker_threads, 
                                      max_threads, max_connections);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_poll_fds_create(&ns_websocket_context.poll_fds, NS_WEBSOCKET, max_connections);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_worker_threads_add_work(&ns_websocket_context.worker_threads,
                                        ns_websocket_receiver_thread_entry, NULL);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    return NS_SUCCESS;
}

int 
ns_websocket_listen(NsWebSocket *websocket, const char *port)
{
    int status;

    if(ns_socket_listen(&websocket->socket, port) != NS_SUCCESS)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    return NS_SUCCESS;
}

int 
ns_websocket_close(NsWebSocket *websocket)
{
    int status;
    NsSocket *socket = &websocket->socket;

    // must remove() before close() because ns_socket_get_bytes_available()
    status = ns_poll_fds_remove(&ns_websocket_context.poll_fds, websocket);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_socket_close(&websocket->socket);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    return NS_SUCCESS;
}

int 
ns_websocket_get_peer(NsWebSocket *websocket, NsWebSocket *peer_websocket)
{
    int status;

    if(ns_poll_fds_is_full(&ns_websocket_context.poll_fds))
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    NsSocket *peer_socket = &peer_websocket->socket;
    status = ns_socket_get_peer(&websocket->socket, peer_socket);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    char peer_handshake[4096];
    int peer_handshake_length = 0;
    {
        int peer_handshake_length = ns_socket_receive(peer_socket, peer_handshake, sizeof(peer_handshake));
        if(peer_handshake_length <= 0)
        {
            DebugPrintInfo();
            return peer_handshake_length;
        }

        peer_handshake[peer_handshake_length] = 0;
    }

    char peer_handshake_reply[512] =
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: ";
    {
        char reply_key[128];
        {
            char peer_key[128];
            {
                char *start_of_key = NULL;
                for(unsigned int i = 0; i < (peer_handshake_length - NS_WEBSOCKET_KEY_HEADER_LENGTH); i++)
                {
                    if(!strncmp(&peer_handshake[i], NS_WEBSOCKET_KEY_HEADER, NS_WEBSOCKET_KEY_HEADER_LENGTH))
                    {
                        start_of_key = &peer_handshake[i + NS_WEBSOCKET_KEY_HEADER_LENGTH];
                        break;
                    }
                }

                // place key in buffer
                int length = 0;
                while(*start_of_key != '\r')
                {
                    peer_key[length++] = *start_of_key++;
                }
                peer_key[length] = 0;
            }

            strcat(peer_key, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

            status = ns_sha1(peer_key, reply_key);
            if(status != NS_SUCCESS)
            {
                DebugPrintInfo();
                return status;
            }
        }

        // fill out key for our handshake reply
        int handshake_reply_length = strlen(peer_handshake_reply);
        strcat(&peer_handshake_reply[handshake_reply_length], reply_key);
        handshake_reply_length += strlen(reply_key);
        peer_handshake_reply[handshake_reply_length++] = '\r';
        peer_handshake_reply[handshake_reply_length++] = '\n';
        peer_handshake_reply[handshake_reply_length++] = '\r';
        peer_handshake_reply[handshake_reply_length++] = '\n';
        peer_handshake_reply[handshake_reply_length++] = 0;
    }

    int bytes_sent = ns_socket_send(peer_socket, peer_handshake_reply, strlen(peer_handshake_reply));
    if(bytes_sent <= 0) 
    {
        DebugPrintInfo();
        return bytes_sent;
    }

    status = ns_poll_fds_add(&ns_websocket_context.poll_fds, peer_websocket);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    return NS_SUCCESS;
}

int 
ns_websocket_receive(NsWebSocket *websocket, uint8_t *dest, uint32_t dest_size)
{
    int message_length = ns_websocket_message_get(websocket, dest, dest_size);
    if(message_length <= 0)
    {
        DebugPrintInfo();
        return message_length;
    }
    return message_length;
}

int 
ns_websocket_receive(NsWebSocket *websocket, char *dest, uint32_t dest_size)
{
    int message_length = ns_websocket_receive(websocket, (uint8_t *)dest, dest_size);
    return message_length;
}

int 
ns_websocket_send(NsWebSocket *websocket, uint8_t *message, uint32_t message_length)
{
    NsSocket *socket = &websocket->socket;

    uint8_t frame[Kilobytes(4)] = {};
    if(message_length >= sizeof(frame))
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    // fill out frame
    int bytes_received;
    {
        // set fin bit
        frame[0] |= 0x80;

        // set opcode to text
        frame[0] |= 0x01;

        // set payload length
        uint8_t *payload;
        if(message_length <= 125)
        {
            frame[1] |= message_length;
            payload = &frame[2];
            bytes_received = 2;
        }
        else
        {
            ns_put64be(&frame[2], message_length);
            payload = &frame[10];
            bytes_received = 10;
        }

        // copy payload
        memcpy(payload, message, message_length);
        bytes_received += message_length;
    }

    int bytes_sent = ns_socket_send(socket, frame, bytes_received);
    if(bytes_sent != bytes_received)
    {
        if(bytes_sent == NS_SOCKET_CONNECTION_CLOSED)
        {
            return 0;
        }

        DebugPrintInfo();
        return bytes_sent;
    }

    return message_length;
}

int 
ns_websocket_send(NsWebSocket *websocket, char *message, uint32_t message_length)
{
    int bytes_sent = ns_websocket_send(websocket, (uint8_t *)message, message_length);
    return bytes_sent;
}

#endif
