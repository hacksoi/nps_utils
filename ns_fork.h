#ifndef NS_FORK_H
#define NS_FORK_H

#include "ns_common.h"

#if defined(WINDOWS)
#elif defined(LINUX)
    #include <unistd.h>
#endif


/* API */

/* Spawns a process and sets its stdout to point to our stdin. */
int
ns_fork_process(const char *filename, char *const *argv, int *child_pid = NULL)
{
    int status;

#if defined(WINDOWS)
#elif defined(LINUX)
    // create pipe
    int fd[2];
    status = pipe(fd);
    if(status == -1)
    {
        DebugPrintOsInfo();
        return NS_ERROR;
    }

    int pipe_read = fd[0];
    int pipe_write = fd[1];

    // fork the fuck out of the child
    int fork_result = fork();
    if(fork_result < 0)
    {
        DebugPrintOsInfo();
        return NS_ERROR;
    }

    // are we the parent?
    if(fork_result > 0)
    {
        status = close(pipe_write);
        if(status == -1)
        {
            DebugPrintOsInfo();
            return NS_ERROR;
        }

        status = dup2(pipe_read, STDIN_FILENO);
        if(status == -1)
        {
            DebugPrintOsInfo();
            return NS_ERROR;
        }

        if(child_pid != NULL)
        {
            *child_pid = fork_result;
        }
    }
    else
    {
        status = close(pipe_read);
        if(status == -1)
        {
            DebugPrintOsInfo();
            return NS_ERROR;
        }

        // set child's stdout to pipe
        status = dup2(pipe_write, STDOUT_FILENO);
        if(status == -1)
        {
            DebugPrintOsInfo();
            return NS_ERROR;
        }

        // exec child
        status = execvp(filename, argv);
        if(status == -1)
        {
            DebugPrintOsInfo();
            return NS_ERROR;
        }
    }
#endif

    return NS_SUCCESS;
}

#endif
