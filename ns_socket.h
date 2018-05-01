#if 0
TODO: change 'peer' to 'peer'
#endif

#ifndef NS_SOCKET_H
#define NS_SOCKET_H

#include "ns_common.h"
#include "ns_pollfd.h"

#if defined(WINDOWS)
    #include <winsock2.h>
    #include <Ws2tcpip.h>

    #pragma comment(lib, "Ws2_32.lib")
#elif defined(LINUX)
    #include <errno.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <sys/ioctl.h>
#endif

#include <string.h>


#if defined(WINDOWS)
    typedef SOCKET NsInternalSocket;

    #define NS_INVALID_SOCKET INVALID_SOCKET
    #define NS_SOCKET_ERROR SOCKET_ERROR
    #define NS_SOCKET_RDWR SD_BOTH
    #define DebugSocketPrintInfo() DebugSocketPrintInfo(); fprintf(stderr, "    socket error code: %d", WSAGetLastError());
#elif defined(LINUX)
    typedef int NsInternalSocket;

    #define NS_INVALID_SOCKET -1
    #define NS_SOCKET_ERROR -1
    #define NS_SOCKET_PEER_CLOSED 0

    // send()
    #define NS_SOCKET_SEND_MSG_NOSIGNAL MSG_NOSIGNAL 

    // shutdown()
    #define NS_SOCKET_SHUT_RDWR SHUT_RDWR

    // ioctl()
    #define NS_SOCKET_IOCTL_FIONREAD FIONREAD

    #define DebugSocketPrintInfo() DebugPrintInfo(); perror("    socket error")
#endif

#define NS_SOCKET_CONNECTION_CLOSED -2
#define NS_SOCKET_BAD_FD -3


struct NsSocket
{
    NsInternalSocket internal_socket;

    void *(*completion_callback)(NsSocket *);
    void *extra_data_void_ptr;
};


/* Internal */

// get sockaddr, IPv4 or IPv6:
internal void *
ns_get_in_addr(struct sockaddr *sa)
{
	if(sa->sa_family == AF_INET)
    {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

internal int 
ns_socket_listen(NsSocket *ns_socket, const char *port, int backlog, 
                 void *(*completion_callback)(NsSocket *), void *extra_data_void_ptr)
{
    int status;

	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

    addrinfo *servinfo;
    status = getaddrinfo(NULL, port, &hints, &servinfo);
	if(status != 0)
    {
        DebugSocketPrintInfo();
		return NS_ERROR;
	}

    NsInternalSocket internal_socket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if(internal_socket == NS_INVALID_SOCKET)
    {
        DebugSocketPrintInfo();
        return NS_ERROR;
    }

    int yes = 1;
    if(setsockopt(internal_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(int)) == NS_SOCKET_ERROR)
    {
        DebugSocketPrintInfo();
        return NS_ERROR;
    }

    if(bind(internal_socket, servinfo->ai_addr, servinfo->ai_addrlen) == NS_SOCKET_ERROR)
    {
        DebugSocketPrintInfo();
        return NS_ERROR;
    }

	freeaddrinfo(servinfo); // all done with this structure

	if(listen(internal_socket, backlog) == NS_SOCKET_ERROR)
    {
        DebugSocketPrintInfo();
        return NS_ERROR;
	}

    ns_socket->internal_socket = internal_socket;
    ns_socket->completion_callback = completion_callback;
    ns_socket->extra_data_void_ptr = extra_data_void_ptr;

    return NS_SUCCESS;
}

internal NsInternalSocket
ns_socket_get_internal(NsSocket *socket)
{
    NsInternalSocket internal_socket = socket->internal_socket;
    return internal_socket;
}

/* API */

int 
ns_sockets_startup()
{
#if defined(WINDOWS)
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return NS_ERROR;
    }
#elif defined(LINUX)
#endif
    return NS_SUCCESS;
}

int 
ns_socket_poll(NsPollFd *fds, nfds_t nfds, int timeout)
{
    int num_fds_ready;
#if defined(WINDOWS)
#elif defined(LINUX)
    num_fds_ready = poll(fds, nfds, timeout);
    if(num_fds_ready == -1)
    {
        DebugSocketPrintInfo();
        return NS_ERROR;
    }
#endif
    return num_fds_ready;
}

int
ns_socket_get_bytes_available(NsSocket *socket)
{
    int bytes_available;
#if defined(WINDOWS)
#elif defined(LINUX)
    int status = ioctl(socket->internal_socket, NS_SOCKET_IOCTL_FIONREAD, &bytes_available);
    if(status == -1)
    {
        DebugSocketPrintInfo();
        printf("    internal socket: %d\n", socket->internal_socket);

        if(errno == EBADF)
        {
            return NS_SOCKET_BAD_FD;
        }

        return NS_ERROR;
    }
#endif
    return bytes_available;
}

int 
ns_socket_listen(NsSocket *ns_socket, const char *port, int backlog = 10)
{
    int status = ns_socket_listen(ns_socket, port, backlog, NULL, NULL);
    return status;
}

int 
ns_socket_close(NsSocket *socket)
{
    int status;

    status = close(socket->internal_socket);
    if(status == -1)
    {
        DebugSocketPrintInfo();
        return NS_ERROR;
    }

    if(socket->completion_callback != NULL)
    {
        socket->completion_callback(socket);
    }

    return NS_SUCCESS;
}

int 
ns_socket_accept(NsSocket *ns_socket, NsSocket *peer_socket, uint32_t timeout_millis = 0, const char *name = NULL)
{
    NsInternalSocket internal_socket = ns_socket->internal_socket;

    if(timeout_millis > 0)
    {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(internal_socket, &rfds);

        timeval tv = {};
        tv.tv_usec = 1000*timeout_millis;

        int select_result = select((internal_socket + 1), &rfds, NULL, NULL, &tv);
        if(select_result == -1)
        {
            DebugSocketPrintInfo();
            return NS_ERROR;
        }

        if(select_result == 0)
        {
            return NS_TIMED_OUT;
        }
    }

    sockaddr_storage their_addr;
    socklen_t sin_size = sizeof(their_addr);
    NsInternalSocket internal_peer_socket = accept(internal_socket, (sockaddr *)&their_addr, &sin_size);
    if(internal_peer_socket == NS_INVALID_SOCKET)
    {
        DebugSocketPrintInfo();
        return NS_ERROR;
    }

    if(name != NULL)
    {
        char s[INET6_ADDRSTRLEN];
        inet_ntop(their_addr.ss_family, ns_get_in_addr((sockaddr *)&their_addr), s, sizeof(s));
        printf("%s: got connection from %s\n", name, s);
    }

    peer_socket->internal_socket = internal_peer_socket;

    return NS_SUCCESS;
}

int 
ns_socket_send(NsSocket *socket, char *buffer, uint32_t buffer_size)
{
    int bytes_sent = send(socket->internal_socket, buffer, buffer_size, NS_SOCKET_SEND_MSG_NOSIGNAL);
    if(bytes_sent < 0)
    {
        if(errno == EPIPE || // EPIPE means the peer closed gracefully
           errno == ECONNRESET // ECONNRESET means the peer didn't close gracefully
           )
        {
            return NS_SOCKET_CONNECTION_CLOSED;
        }
        else
        {
            DebugSocketPrintInfo();
            printf("    fd: %d\n", socket->internal_socket);
        }
    }
    return bytes_sent;
}

int 
ns_socket_send(NsSocket *socket, uint8_t *buffer, uint32_t buffer_size)
{
    int bytes_sent = ns_socket_send(socket, (char *)buffer, buffer_size);
    return bytes_sent;
}

int 
ns_socket_receive(NsSocket *socket, char *buffer, uint32_t buffer_size)
{
    int bytes_received = recv(socket->internal_socket, buffer, buffer_size, 0);
    if(bytes_received < 0)
    {
        DebugSocketPrintInfo();
        printf("    fd: %d\n", socket->internal_socket);
    }
    return bytes_received;
}

int 
ns_socket_receive(NsSocket *socket, uint8_t *buffer, uint32_t buffer_size)
{
    int bytes_received = ns_socket_receive(socket, (char *)buffer, buffer_size);
    return bytes_received;
}

int 
ns_socket_shutdown(NsSocket *socket, int how)
{
#if defined(WINDOWS)
#elif defined(LINUX)
    if(shutdown(socket->internal_socket, how))
    {
        DebugSocketPrintInfo();
        return NS_ERROR;
    }
#endif
    return NS_SUCCESS;
}

#endif
