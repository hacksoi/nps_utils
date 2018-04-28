#ifndef NS_MEMORY_H
#define NS_MEMORY_H

#include <stdlib.h>


void *
ns_memory_allocate(int size)
{
    return malloc(size);
}

void
ns_memory_free(void *memory)
{
    free(memory);
}

#endif
