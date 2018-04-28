#ifndef NS_HTTP_SERVER_H
#define NS_HTTP_SERVER_H

#include "ns_common.h"
#include "ns_socket.h"
#include "ns_thread.h"
#include "ns_file.h"
#include "ns_string.h"
#include "ns_socket_pool.h"
#include "ns_worker_threads.h"


struct NsHttpServer
{
    int max_connections;
    const char *port;
    NsWorkerThreads worker_threads;
};


/* Internal */

internal void *
ns_http_server_client_thread_entry(void *thread_input)
{
    int status;
    NsSocket *socket = (NsSocket *)thread_input;

    while(1)
    {
        char buf[1024]; // TODO: len?
        int bytes_received = ns_socket_receive(socket, buf, sizeof(buf));
        if(bytes_received <= 0)
        {
            DebugPrintInfo();
            exit(1);
        }

        char token[256]; // TODO: len?

        // request
        int len = ns_string_get_token(token, buf, sizeof(token), ' ');
        if(len <= 0)
        {
            DebugPrintInfo();
            return (void *)len;
        }

        if(!strcmp(token, "GET"))
        {
            // resource
            len = ns_string_get_token(token, buf + len, sizeof(token), ' ');
            if(len <= 0)
            {
                DebugPrintInfo();
                return (void *)len;
            }

            char *resource_filename = !strcmp(token, "/") ? (char *)"index.html" : token;
            if(ns_file_check_exists(resource_filename))
            {
                char resource[Kilobytes(4)]; // TODO: len?
                int resource_size;
                {
                    NsFile file;
                    status = ns_file_open(&file, resource_filename);
                    if(status != NS_SUCCESS)
                    {
                        DebugPrintInfo();
                        exit(1);
                    }

                    resource_size = ns_file_load(&file, resource, sizeof(resource));
                    if(resource_size <= 0)
                    {
                        DebugPrintInfo();
                        exit(1);
                    }

                    status = ns_file_close(&file);
                    if(status != NS_SUCCESS)
                    {
                        DebugPrintInfo();
                        exit(1);
                    }
                }

                int bytes_sent = ns_socket_send(socket, resource, resource_size);
                if(bytes_sent <= 0) 
                {
                    DebugPrintInfo();
                    exit(1);
                }
            }
            else
            {
                // send 404
            }
        }
        else
        {
            // TODO
        }
    }
}

internal void *
ns_http_server_client_receiver_thread_entry(void *thread_input)
{
}

internal void *
ns_http_server_client_getter_thread_entry(void *thread_input)
{
    int status;
    NsHttpServer *http_server = (NsHttpServer *)thread_input;

    NsSocket socket;
    status = ns_socket_listen(&socket, http_server->port);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return (void *)status;
    }

	printf("http server: waiting for connections...\n");

    while(1)
    {
        status = ns_socket_pool_socket_get(&socket_pool, &socket, &client_socket, 0, "http server");
        if(status != NS_SUCCESS)
        {
            DebugPrintInfo();
            return status;
        }

        status = ns_socket_get_client(&socket, &client_socket, 0, "http server");
        if(status != NS_SUCCESS)
        {
            DebugPrintInfo();
            return (void *)status;
        }

        // add client to pollfds

        status = ns_thread_pool_create_thread(&ns_http_server_thread_pool, 
                                              ns_http_server_client_thread_entry, (void *)client_socket);
        if(status != NS_SUCCESS)
        {
            DebugPrintInfo();
            return (void *)status;
        }
    }
}


/* API */

int
ns_http_server_create(NsHttpServer *http_server, int max_connections, const char *port = "80")
{
    int status;

    status = ns_worker_threads_create(&http_server->worker_threads, max_connections);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    status = ns_socket_pool_create(&ns_http_server_socket_pool, max_connections);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    http_server->max_connections = max_connections;
    http_server->port = port;

    status = ns_worker_threads_add_work(&ns_http_server_thread_pool, 
                                        ns_http_server_client_getter_thread_entry, http_server);
    if(status != NS_SUCCESS)
    {
        DebugPrintInfo();
        return status;
    }

    return NS_SUCCESS;
}

#endif
