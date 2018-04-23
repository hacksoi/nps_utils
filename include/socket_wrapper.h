#ifndef NS_SOCKET_WRAPPER_H
#define NS_SOCKET_WRAPPER_H

#include "common.h"

#if defined(WINDOWS)
    #if !defined(WIN32_LEAN_AND_MEAN)
        #define WIN32_LEAN_AND_MEAN
    #endif

    #include <winsock2.h>
    #include <Ws2tcpip.h>

    #define SOCKET_TYPE SOCKET
    //#define INVALID_SOCKET INVALID_SOCKET
    //#define SOCKET_ERROR SOCKET_ERROR
#elif defined(LINUX)
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <arpa/inet.h>

    #define SOCKET_TYPE int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

#define TIMED_OUT -2

/* Structs */
//{

struct Socket
{
    SOCKET_TYPE internal_socket;
};
//}

/* Functions */
//{

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if(sa->sa_family == AF_INET)
    {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

bool create_socket(Socket *socket, const char *port, int backlog = 10)
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
        DEBUG_PRINT_INFO();
		return false;
	}

    SocketType internal_socket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if(internal_socket == INVALID_SOCKET)
    {
        DEBUG_PRINT_INFO();
        return false;
    }

    int yes = 1;
    if(setsockopt(socket->internal_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == SOCKET_ERROR)
    {
        DEBUG_PRINT_INFO();
        return false;
    }

    if(bind(socket->internal_socket, servinfo->ai_addr, servinfo->ai_addrlen) == SOCKET_ERROR)
    {
        DEBUG_PRINT_INFO();
        return false;
    }

	freeaddrinfo(servinfo); // all done with this structure

	if(listen(socket->internal_socket, backlog) == SOCKET_ERROR)
    {
        DEBUG_PRINT_INFO();
        return false;
	}

    socket->internal_socket = internal_socket;

    return true;
}

int get_client(int sock_fd, uint32_t timeout_millis = 0, const char *name = NULL)
{
    if(timeout_millis > 0)
    {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(sock_fd, &rfds);

        timeval tv = {};
        tv.tv_usec = 1000*timeout_millis;

        int select_result = select((sock_fd + 1), &rfds, NULL, NULL, &tv);
        if(select_result == -1)
        {
            DEBUG_PRINT_INFO();
            return -1;
        }

        if(select_result == 0)
        {
            return -2;
        }
    }

    sockaddr_storage their_addr;
    socklen_t sin_size = sizeof(their_addr);
    int client_fd = accept(sock_fd, (sockaddr *)&their_addr, &sin_size);
    if(client_fd == -1)
    {
        DEBUG_PRINT_INFO();
        return -1;
    }

    if(name != NULL)
    {
        char s[INET6_ADDRSTRLEN];
        inet_ntop(their_addr.ss_family, get_in_addr((sockaddr *)&their_addr), s, sizeof(s));
        printf("%s: got connection from %s\n", name, s);
    }

    return client_fd;
}
//}

#endif
