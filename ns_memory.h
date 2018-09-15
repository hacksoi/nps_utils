#ifndef NS_MEMORY_H
#define NS_MEMORY_H

#include "ns_common.h"

/* @race_condition */
internal uint8_t GlobalStackMemory[Megabytes(512)];
internal uint8_t *GlobalEndStackMemory = GlobalStackMemory + sizeof(GlobalStackMemory);
internal uint8_t *GlobalCurStackMemory = GlobalStackMemory;
internal uint32_t GlobalDebugAllocs[2048];
internal uint32_t GlobalDebugNumAllocs;

internal uint8_t *
PushPerFrameMemory(uint32_t NumBytes)
{
    uint8_t *Result = GlobalCurStackMemory;
    GlobalCurStackMemory += NumBytes;
    *(uint32_t *)GlobalCurStackMemory = NumBytes;
    GlobalCurStackMemory += sizeof(NumBytes);
    Assert(GlobalCurStackMemory <= GlobalEndStackMemory);
    GlobalDebugAllocs[GlobalDebugNumAllocs++] = NumBytes;
    return Result;
}

internal void
PopPerFrameMemory()
{
    GlobalCurStackMemory -= sizeof(uint32_t);
    uint32_t PopSize = *(uint32_t *)GlobalCurStackMemory;
	Assert(GlobalDebugNumAllocs > 0);
    Assert(PopSize == GlobalDebugAllocs[--GlobalDebugNumAllocs]);
    Assert((uint64_t)GlobalCurStackMemory > (uint64_t)PopSize);
    GlobalCurStackMemory -= PopSize;
    Assert(GlobalCurStackMemory >= GlobalStackMemory);
}

internal void
ResetPerFrameMemory()
{
    GlobalCurStackMemory = GlobalStackMemory;
    GlobalDebugNumAllocs = 0;
}

internal uint8_t *
MemAlloc(int Size)
{
    uint8_t *Result = (uint8_t *)malloc(Size);
    return Result;
}

internal uint8_t *
MemAllocZero(int Size)
{
    uint8_t *Result = (uint8_t *)calloc(1, Size);
    return Result;
}

internal void
MemFree(void *MemPtr)
{
    free(MemPtr);
}

internal void
MemCpy(void *Destination, void *Source, int Size)
{
    memcpy(Destination, Source, Size);
}

internal int
MemCmp(void *Destination, void *Source, int Size)
{
    int Result = memcmp(Destination, Source, Size);
    return Result;
}

internal void
MemSet0(void *Memory, int Size)
{
    memset(Memory, 0, Size);
}

#endif
