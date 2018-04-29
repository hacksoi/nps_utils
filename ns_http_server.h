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
ns_http_server_client_thread_entry(void *thread_input)
{
    int status;

    uint8_t *mem = (uint8_t *)thread_input;
    NsSocket *socket = *(NsSocket **)mem;
    char *buf = (char *)((NsSocket **)mem + 1);

    char token[256]; // TODO: len?

    // get request
    int len = ns_string_get_token(token, buf, sizeof(token), ' ');
    if(len > 0)
    {
        buf += (len + 1);
        if(!strcmp(token, "GET"))
        {
            // get resource
            len = ns_string_get_token(token, buf, sizeof(token), ' ');
            if(len > 0)
            {
                char *resource_filename = !strcmp(token, "/") ? (char *)"index.html" : token;
                if(ns_file_check_exists(resource_filename))
                {
                    printf("%s\n", resource_filename);

                    char resource[Kilobytes(4)]; // TODO: len?
                    int resource_size;

                    NsFile file;
                    status = ns_file_open(&file, resource_filename);
                    if(status == NS_SUCCESS)
                    {
                        resource_size = ns_file_load(&file, resource, sizeof(resource));
                        if(resource_size > 0)
                        {
                            status = ns_file_close(&file);
                            if(status == NS_SUCCESS)
                            {
                                printf("sending...\n");
                                int bytes_sent = ns_socket_send(socket, resource, resource_size);
                                if(bytes_sent <= 0) 
                                {
                                    DebugPrintInfo();
                                }
                                printf("sent!...\n");
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
                    printf("http server: unknown resource request: %s\n", resource_filename);
                }
            }
            else
            {
                DebugPrintInfo();
            }
        }
        else
        {
            // TODO
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
ns_http_server_client_receiver_thread_entry(void *thread_input)
{
    int status;

    while(1)
    {
        status = ns_poll_fds_wait_till_nonempty_and_lock(&ns_http_server_context.poll_fds);
        if(status != NS_SUCCESS)
        {
            DebugPrintInfo();
            return (void *)status;
        }

        NsPollFd *pollfds = ns_poll_fds_get(&ns_http_server_context.poll_fds);
        int pollfds_capacity = ns_poll_fds_get_capacity(&ns_http_server_context.poll_fds);

        int num_fds_ready = ns_socket_poll(pollfds, pollfds_capacity, 10);
        if(num_fds_ready < 0)
        {
            DebugPrintInfo();
            return (void *)NS_ERROR;
        }

        if(num_fds_ready > 0)
        {
            for(int i = 0; i < pollfds_capacity; i++)
            {
                if(pollfds[i].fd >= 0)
                {
                    NsSocket *socket = (NsSocket *)ns_poll_fds_get_container(&ns_http_server_context.poll_fds, i);

                    // is this socket ready for reading?
                    if((pollfds[i].revents & NS_SOCKET_POLL_IN) != 0)
                    {
                        printf("holy moly\n");

                        int message_size = ns_socket_get_bytes_available(socket);

                        // sanity check
                        if(message_size <= 0)
                        {
                            printf("ms: %d\n", message_size);
                            DebugPrintInfo();
                            return (void *)NS_ERROR;
                        }

                        uint8_t *mem = (uint8_t *)ns_memory_allocate(sizeof(NsSocket *) + message_size);
                        *(NsSocket **)mem = socket;
                        uint8_t *buf = (uint8_t *)((NsSocket **)mem + 1);

                        int bytes_received = ns_socket_receive(socket, buf, message_size);
                        if(bytes_received == message_size)
                        {
                            for(int i = 0; i < message_size; i++)
                            {
                                putchar(buf[i]);
                            }

                            // sanity check
                            if(bytes_received != message_size)
                            {
                                DebugPrintInfo();
                                return (void *)NS_ERROR;
                            }

                            status = ns_worker_threads_add_work(&ns_http_server_context.worker_threads, 
                                                                ns_http_server_client_thread_entry, mem);
                            if(status != NS_SUCCESS)
                            {
                                DebugPrintInfo();
                                return (void *)status;
                            }
                        }
                        else
                        {
                            // peer may have closed connection, so don't return
                            DebugPrintInfo();
                            ns_memory_free(mem);
                        }
                    }
                    // peer closed connection?
                    else if((pollfds[i].revents & NS_SOCKET_POLL_HUP) != 0)
                    {
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
                    }
                }
            }
        }

        status = ns_poll_fds_unlock(&ns_http_server_context.poll_fds);
        if(status != NS_SUCCESS)
        {
            DebugPrintInfo();
            return (void *)status;
        }
    }

    return (void *)NS_SUCCESS;
}

internal void *
ns_http_server_client_getter_thread_entry(void *thread_input)
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
        NsSocket *client_socket;
        status = ns_socket_pool_get(&ns_http_server_context.socket_pool, &client_socket);
        if(status != NS_SUCCESS)
        {
            DebugPrintInfo();
            return (void *)status;
        }

        status = ns_socket_get_client(&socket, client_socket, 0, "http server");
        if(status != NS_SUCCESS)
        {
            DebugPrintInfo();
            return (void *)status;
        }

        status = ns_poll_fds_add(&ns_http_server_context.poll_fds, client_socket);
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

    // we need at least 3 threads: getting clients, receiving requests, and processing requests
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
                                        ns_http_server_client_getter_thread_entry, NULL);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_worker_threads_add_work(&ns_http_server_context.worker_threads, 
                                        ns_http_server_client_receiver_thread_entry, NULL);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    return NS_SUCCESS;
}

#endif
