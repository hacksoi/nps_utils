#ifndef NS_HTTP_SERVER_H
#define NS_HTTP_SERVER_H

#include "ns_common.h"
#include "ns_socket.h"
#include "ns_thread.h"
#include "ns_file.h"
#include "ns_string.h"
#include "ns_socket_pool.h"
#include "ns_worker_threads.h"
#include "ns_pollfd.h"
#include "ns_poll_fds.h"


struct NsHttpServer
{
    int max_connections;
    const char *port;
    NsSocketPool socket_pool;
    NsWorkerThreads worker_threads;
    NsPollFds poll_fds;
};


global NsHttpServer ns_http_server_context;


/* Internal */

internal void *
ns_http_server_peer_thread_entry(void *thread_input)
{
    int status;
    char token[256]; // TODO: len?

    uint8_t *mem = (uint8_t *)thread_input;
    NsSocket *socket = *(NsSocket **)mem;
    char *peer_request = (char *)((NsSocket **)mem + 1);

    // get request
    int len = ns_string_get_token(token, peer_request, sizeof(token), ' ');
    if(len > 0)
    {
        peer_request += (len + 1);

        if(!strcmp(token, "GET"))
        {
            // get resource
            len = ns_string_get_token(token, peer_request, sizeof(token), ' ');
            if(len > 0)
            {
                const char *header_status;
                char *resource_filename = !strcmp(token, "/") ? (char *)"index.html" : token;
                if(ns_file_check_exists(resource_filename))
                {
                    header_status = "HTTP/1.1 200 OK\r\n";
                }
                else
                {
                    header_status = "HTTP/1.1 404 Not Found\r\n";
                    resource_filename = (char *)"404.html";
                }

                NsFile file;
                status = ns_file_open(&file, resource_filename);
                if(status == NS_SUCCESS)
                {
                    int resource_size = ns_file_get_size(&file);
                    if(resource_size > 0)
                    {
                        // construct response

                        char response[Kilobytes(4)]; // TODO: len?
                        int response_length = 0;

                        strcpy(&response[response_length], header_status);
                        response_length += strlen(header_status);

                        const char *header_boiler_plate = 
                            "Connection: keep-alive\r\n"
                            "Content-Type: text/html\r\n" // TODO: handle more than just text/html
                            "Pragma: no-cache\r\n"
                            "Cache-Control: no-cache\r\n"
                            "Content-Length: ";
                        strcpy(&response[response_length], header_boiler_plate);
                        response_length += strlen(header_boiler_plate);

                        int resource_size_length = ns_string_from_int(&response[response_length], resource_size);
                        response_length += resource_size_length;

                        const char *end = "\r\n\r\n";
                        strcpy(&response[response_length], end);
                        response_length += strlen(end);

                        int bytes_read = ns_file_load(&file, &response[response_length], sizeof(response) - response_length);
                        if(bytes_read == resource_size)
                        {
                            response_length += resource_size;

                            status = ns_file_close(&file);
                            if(status == NS_SUCCESS)
                            {
                                int bytes_sent = ns_socket_send(socket, response, response_length);
                                if(bytes_sent != response_length) 
                                {
                                    DebugPrintInfo();
                                }
                            }
                            else
                            {
                                DebugPrintInfo();
                            }
                        }
                        else
                        {
                            DebugPrintInfo();
                        }
                    }
                    else
                    {
                        DebugPrintInfo();
                    }
                }
                else
                {
                    DebugPrintInfo();
                }
            }
            else
            {
                DebugPrintInfo();
            }
        }
        else
        {
            printf("unknown request\n");
        }
    }
    else
    {
        DebugPrintInfo();
    }

    ns_memory_free(mem);

    return (void *)NS_SUCCESS;
}

internal void *
ns_http_server_peer_receiver_thread_entry(void *thread_input)
{
    int status;

    while(1)
    {
        NsPollFd *pollfds = ns_poll_fds_get(&ns_http_server_context.poll_fds);
        int pollfds_capacity = ns_poll_fds_get_capacity(&ns_http_server_context.poll_fds);

        status = ns_poll_fds_wait_till_nonempty_and_lock(&ns_http_server_context.poll_fds);
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

        status = ns_poll_fds_unlock(&ns_http_server_context.poll_fds);
        if(status != NS_SUCCESS)
        {
            DebugPrintInfo();
            return (void *)status;
        }

        if(num_fds_ready > 0)
        {
            for(int i = 0; i < pollfds_capacity; i++)
            {
                if(pollfds[i].fd >= 0)
                {
                    if((pollfds[i].revents & NS_SOCKET_POLL_IN) != 0)
                    {
                        NsSocket *socket = (NsSocket *)ns_poll_fds_get_container(&ns_http_server_context.poll_fds, i);

                        bool closed = false;
                        int message_size = ns_socket_get_bytes_available(socket);
                        if(message_size > 0)
                        {
                            uint8_t *mem = (uint8_t *)ns_memory_allocate(sizeof(NsSocket *) + message_size);
                            *(NsSocket **)mem = socket;
                            uint8_t *buf = (uint8_t *)((NsSocket **)mem + 1);

                            int bytes_received = ns_socket_receive(socket, buf, message_size);
                            if(bytes_received == message_size)
                            {
                                // sanity check
                                if(bytes_received != message_size)
                                {
                                    DebugPrintInfo();
                                    return (void *)NS_ERROR;
                                }

                                status = ns_worker_threads_add_work(&ns_http_server_context.worker_threads, 
                                                                    ns_http_server_peer_thread_entry, mem);
                                if(status != NS_SUCCESS)
                                {
                                    DebugPrintInfo();
                                    return (void *)status;
                                }
                            }
                            else if(bytes_received == 0)
                            {
                                closed = true;
                            }
                            else
                            {
                                DebugPrintInfo();
                                ns_memory_free(mem);
                                return (void *)bytes_received;
                            }
                        }
                        else if(message_size == 0)
                        {
                            closed = true;
                        }
                        else
                        {
                            DebugPrintInfo();
                            return (void *)message_size;
                        }

                        if(closed)
                        {
                            printf("http server: connection closed\n");

                            status = ns_socket_close(socket);
                            if(status != NS_SUCCESS)
                            {
                                DebugPrintInfo();
                                return (void *)status;
                            }

                            status = ns_socket_pool_release(&ns_http_server_context.socket_pool, socket);
                            if(status != NS_SUCCESS)
                            {
                                DebugPrintInfo();
                                return (void *)status;
                            }

                            status = ns_poll_fds_remove(&ns_http_server_context.poll_fds, socket);
                            if(status != NS_SUCCESS)
                            {
                                DebugPrintInfo();
                                return (void *)status;
                            }
                        }
                    }
                }
            }
        }
    }

    return (void *)NS_SUCCESS;
}

internal void *
ns_http_server_peer_getter_thread_entry(void *thread_input)
{
    int status;

    NsSocket socket;
    status = ns_socket_listen(&socket, ns_http_server_context.port);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return (void *)status;
    }

	printf("http server: waiting for connections...\n");

    while(1)
    {
        NsSocket *peer_socket;
        status = ns_socket_pool_get(&ns_http_server_context.socket_pool, &peer_socket);
        if(status != NS_SUCCESS)
        {
            DebugPrintInfo();
            return (void *)status;
        }

        status = ns_socket_get_peer(&socket, peer_socket, 0, "http server");
        if(status != NS_SUCCESS)
        {
            DebugPrintInfo();
            return (void *)status;
        }

        status = ns_poll_fds_add(&ns_http_server_context.poll_fds, peer_socket);
        if(status != NS_SUCCESS)
        {
            DebugPrintInfo();
            return (void *)status;
        }
    }
}


/* API */

int
ns_http_server_startup(int max_connections, const char *port, int max_threads)
{
    int status;

    // we need at least 3 threads: getting peers, receiving requests, and processing requests
    if(max_threads < 3)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    status = ns_poll_fds_create(&ns_http_server_context.poll_fds, NS_SOCKET, max_connections);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_worker_threads_create(&ns_http_server_context.worker_threads, max_threads, max_connections);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_socket_pool_create(&ns_http_server_context.socket_pool, max_connections);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    ns_http_server_context.max_connections = max_connections;
    ns_http_server_context.port = port;

    status = ns_worker_threads_add_work(&ns_http_server_context.worker_threads, 
                                        ns_http_server_peer_getter_thread_entry, NULL);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_worker_threads_add_work(&ns_http_server_context.worker_threads, 
                                        ns_http_server_peer_receiver_thread_entry, NULL);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    return NS_SUCCESS;
}

#endif
