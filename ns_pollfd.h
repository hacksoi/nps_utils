#ifndef NS_POLLFD_H
#define NS_POLLFD_H

#if defined(WINDOWS)
#elif defined(LINUX)
    #include <poll.h>
#endif


#if defined(WINDOWS)
#elif defined(LINUX)
    typedef pollfd NsPollFd;

    #define NS_SOCKET_POLL_IN POLLIN
    #define NS_SOCKET_POLL_HUP POLLHUP
#endif


void
ns_poll_fds_print(NsPollFd *pollfd)
{
    printf("pollfd: fd: %d, event: %d, revent: %d\n",
           pollfd->fd, pollfd->events, pollfd->revents);
}

#endif
