#ifndef NS_SOCKET_WRAPPER_H
#define NS_SOCKET_WRAPPER_H

#include "ns_common.h"

#if defined(WINDOWS)
    #include <winsock2.h>
    #include <Ws2tcpip.h>

    #pragma comment(lib, "Ws2_32.lib")
#elif defined(LINUX)
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <arpa/inet.h>
#endif


#if defined(WINDOWS)
    typedef SOCKET InternalSocket;
    #define NS_INVALID_SOCKET INVALID_SOCKET
    #define NS_SOCKET_ERROR SOCKET_ERROR
    #define DebugSocketPrintInfo() DebugPrintInfo(); fprintf(stderr, "    socket error code: %d", WSAGetLastError());
#elif defined(LINUX)
    typedef int InternalSocket;
    #define NS_INVALID_SOCKET -1
    #define NS_SOCKET_ERROR -1
    #define DebugSocketPrintInfo() DebugPrintInfo()
#endif


struct NsSocket
{
    InternalSocket internal_socket;
};


#if defined(WINDOWS)
int ns_init_sockets()
{
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return NS_ERROR;
    }
    return NS_SUCCESS;
}
#elif defined(LINUX)
#endif

// get sockaddr, IPv4 or IPv6:
void *ns_get_in_addr(struct sockaddr *sa)
{
	if(sa->sa_family == AF_INET)
    {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int ns_create_socket(NsSocket *ns_socket, const char *port, int backlog = 10)
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

    InternalSocket internal_socket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
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

    return NS_SUCCESS;
}

int ns_get_client(NsSocket *ns_socket, NsSocket *client_socket, uint32_t timeout_millis = 0, const char *name = NULL)
{
    InternalSocket internal_socket = ns_socket->internal_socket;

    if(timeout_millis > 0)
    {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(ns_socket->internal_socket, &rfds);

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
    InternalSocket internal_client_socket = accept(internal_socket, (sockaddr *)&their_addr, &sin_size);
    if(internal_client_socket == INVALID_SOCKET)
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

    client_socket->internal_socket = internal_client_socket;

    return NS_SUCCESS;
}

#endif
