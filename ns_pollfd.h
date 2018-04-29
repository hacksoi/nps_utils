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

#endif
