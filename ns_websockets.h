#if 0
// TODO:
    -instead of making a thread per client (over 1000 threads?) keep a table of clients and iterate over them.
#endif

#ifndef NS_WEBSOCKETS_H
#define NS_WEBSOCKETS_H

#include "ns_common.h"
#include "ns_util.h"
#include "ns_sha1.h"
#include "ns_message_queue.h"
#include "ns_socket_wrapper.h"

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>


#define NS_WEBSOCKET_KEY_HEADER "Sec-WebSocket-Key: "
#define NS_WEBSOCKET_KEY_HEADER_LENGTH strlen(NS_WEBSOCKET_KEY_HEADER)

#define NS_OPCODE_CONNECTION_CLOSE 0x08
#define NS_OPCODE_PING 0x09
#define NS_OPCODE_PONG 0x0A


// TODO: sync this
pthread_t ns_websocket_thread_pool[20];
int ns_websocket_thread_pool_size;


/* Internal */

struct NsWebSocket
{
    NsSocket socket;
    NsMessageQueue message_queue;
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


int ns_websocket_create(NsWebSocket *websocket, const char *port);
int ns_websocket_destroy(NsWebSocket *websocket);
int ns_websocket_get_client(NsWebSocket *websocket, NsWebSocket *client_websocket);
int ns_websocket_receive(NsWebSocket *websocket, uint8_t *dest, uint32_t dest_size);
int ns_websocket_send(NsWebSocket *websocket, uint8_t *message, uint32_t message_length);

void ns_websocket_frame_print(NsWebSocketFrame frame)
{
    printf("fin: 0x%x, rsv1: 0x%x, rsv2: 0x%x, rsv3: 0x%x, opcode: 0x%x, mask: 0x%x, "
           "payload_length: %llu, mask[0]: 0x%x, mask[1]: 0x%x, mask[2]: 0x%x, mask[3]: 0x%x\n", 
           frame.fin, frame.rsv1, frame.rsv2, frame.rsv3, frame.opcode, frame.mask, 
           frame.payload_length, frame.mask_key[0], frame.mask_key[1], frame.mask_key[2], frame.mask_key[3]);
}

NsWebSocketFrame ns_websocket_frame_inflate(uint8_t *raw_frame)
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

int ns_websocket_init(NsWebSocket *websocket, NsSocket *socket)
{
    websocket->socket = *socket;
    if(ns_message_queue_create(&websocket->message_queue) == NS_ERROR)
    {
        return NS_ERROR;
    }

    return NS_SUCCESS;
}

int ns_websocket_send_close(NsWebSocket *websocket)
{
    NsSocket *socket = &websocket->socket;

    uint8_t frame[2] = {};
    int bytes_received;
    {
        // set fin bit
        frame[0] |= 0x80;

        // set opcode to close
        frame[0] |= 0x08;

        bytes_received = 2;
    }

    if(ns_socket_send(socket, frame, bytes_received) == NS_ERROR)
    {
        return NS_ERROR;
    }

    return NS_SUCCESS;
}

void *ns_websocket_background_client_thread_entry(void *thread_data)
{
    NsWebSocket *websocket = (NsWebSocket *)thread_data;
    NsSocket *socket = &websocket->socket;

    while(1)
    {
        uint8_t raw_frame[1024];
        int bytes_received = ns_socket_receive(socket, (char *)raw_frame, sizeof(raw_frame));
        if(bytes_received < 0)
        {
            DebugPrintInfo();
            exit(1);
        }

        NsWebSocketFrame frame = ns_websocket_frame_inflate(raw_frame);
        switch(frame.opcode)
        {
            case NS_OPCODE_CONNECTION_CLOSE:
            {
                printf("received a close\n");
                if(ns_websocket_send_close(websocket) == NS_ERROR)
                {
                    return (void *)NS_ERROR;
                }
                return NS_SUCCESS;
            } break;

            case NS_OPCODE_PING:
            {
                printf("received a ping\n");

                // send pong
                {
                    // change opcode to pong
                    {
                        raw_frame[0] &= ~NS_OPCODE_PING;
                        raw_frame[0] |= NS_OPCODE_PONG;
                    }

                    int bytes_sent = ns_websocket_send(websocket, raw_frame, bytes_received);
                    if(bytes_sent != bytes_received)
                    {
                        DebugPrintInfo();
                        return NS_SUCCESS;
                    }
                }
            } break;

            default:
            {
                // decode
                for(uint32_t i = 0; i < frame.payload_length; i++)
                {
                    frame.payload[i] ^= frame.mask_key[i % 4];
                }

                int bytes_added = ns_message_queue_add(&websocket->message_queue, frame.payload, frame.payload_length);
                if(bytes_added != (int)frame.payload_length)
                {
                    DebugPrintInfo();
                }
            } break;
        }
    }

    return NS_SUCCESS;
}


/* API */

int ns_websocket_create(NsWebSocket *websocket, const char *port)
{
    NsSocket socket;
    if(ns_socket_create(&socket, port) == NS_ERROR)
    {
        return NS_ERROR;
    }

    if(ns_websocket_init(websocket, &socket) == NS_ERROR)
    {
        return NS_ERROR;
    }

    return NS_SUCCESS;
}

int ns_websocket_close(NsWebSocket *websocket)
{
    ns_websocket_send_close(websocket);

    NsSocket *socket = &websocket->socket;

    // receive frames until we receive the close frame response
    while(1)
    {
        uint8_t frame[256];
        if(ns_socket_receive(socket, frame, sizeof(frame)) == NS_ERROR)
        {
            return NS_ERROR;
        }

        if((frame[0] & 0x0f) == 0x08)
        {
            break;
        }
    }

    if(ns_socket_close(socket) == NS_ERROR)
    {
        return NS_ERROR;
    }

    return NS_SUCCESS;
}

int ns_websocket_get_client(NsWebSocket *websocket, NsWebSocket *client_websocket)
{
    NsSocket client_socket;
    if(ns_socket_get_client(&websocket->socket, &client_socket) == NS_ERROR)
    {
        return NS_ERROR;
    }

    char client_handshake[4096];
    int client_handshake_length = 0;
    {
        int client_handshake_length = ns_socket_receive(&client_socket, client_handshake, sizeof(client_handshake));
        if(client_handshake_length == NS_ERROR)
        {
            return NS_ERROR;
        }

        client_handshake[client_handshake_length] = 0;
    }

    char client_handshake_reply[512] =
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: ";
    {
        char reply_key[128];
        {
            char client_key[128];
            {
                char *start_of_key = NULL;
                for(unsigned int i = 0; i < (client_handshake_length - NS_WEBSOCKET_KEY_HEADER_LENGTH); i++)
                {
                    if(!strncmp(&client_handshake[i], NS_WEBSOCKET_KEY_HEADER, NS_WEBSOCKET_KEY_HEADER_LENGTH))
                    {
                        start_of_key = &client_handshake[i + NS_WEBSOCKET_KEY_HEADER_LENGTH];
                        break;
                    }
                }

                // place key in buffer
                int length = 0;
                while(*start_of_key != '\r')
                {
                    client_key[length++] = *start_of_key++;
                }
                client_key[length] = 0;
            }

            strcat(client_key, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

            if(ns_sha1(client_key, reply_key) == NS_ERROR)
            {
                return NS_ERROR;
            }
        }

        // fill out key for our handshake reply
        int handshake_reply_length = strlen(client_handshake_reply);
        strcat(&client_handshake_reply[handshake_reply_length], reply_key);
        handshake_reply_length += strlen(reply_key);
        client_handshake_reply[handshake_reply_length++] = '\r';
        client_handshake_reply[handshake_reply_length++] = '\n';
        client_handshake_reply[handshake_reply_length++] = '\r';
        client_handshake_reply[handshake_reply_length++] = '\n';
        client_handshake_reply[handshake_reply_length++] = 0;
    }

    if(ns_socket_send(&client_socket, client_handshake_reply, strlen(client_handshake_reply)) == NS_ERROR) 
    {
        return NS_ERROR;
    }

    if(ns_websocket_init(client_websocket, &client_socket) == NS_ERROR)
    {
        return NS_ERROR;
    }

    pthread_create(&ns_websocket_thread_pool[ns_websocket_thread_pool_size++], NULL, ns_websocket_background_client_thread_entry, client_websocket);

    return NS_SUCCESS;
}

int ns_websocket_receive(NsWebSocket *websocket, uint8_t *dest, uint32_t dest_size)
{
    int message_length = ns_message_queue_get(&websocket->message_queue, dest, dest_size);
    return message_length;
}

int ns_websocket_receive(NsWebSocket *websocket, char *dest, uint32_t dest_size)
{
    int message_length = ns_websocket_receive(websocket, (uint8_t *)dest, dest_size);
    return message_length;
}

int ns_websocket_send(NsWebSocket *websocket, uint8_t *message, uint32_t message_length)
{
    NsSocket *socket = &websocket->socket;

    uint8_t frame[256] = {};
    assert(message_length < sizeof(frame));

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

    if(ns_socket_send(socket, frame, bytes_received) == NS_SOCKET_ERROR)
    {
        return NS_ERROR;
    }

    return NS_SUCCESS;
}

int ns_websocket_send(NsWebSocket *websocket, char *message, uint32_t message_length)
{
    int bytes_sent = ns_websocket_send(websocket, (uint8_t *)message, message_length);
    return bytes_sent;
}

#endif
