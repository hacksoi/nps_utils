#ifndef NS_FILES_H
#define NS_FILES_H

#include "ns_common.h"
#include "ns_memory.h"

#if 0 /* Just use the standard library FFS. */
#if defined(WINDOWS)
#elif defined(LINUX)
    #include <unistd.h>
#endif

#include <stdlib.h>


#if defined(WINDOWS)
    #define DebugPrintFileInfo() DebugPrintInfo();
#elif defined(LINUX)
    #define DebugPrintFileInfo() DebugPrintInfo(); perror("    file error")
#endif


struct NsFile
{
    FILE *internal_file;
};


bool 
ns_file_check_exists(char *filename)
{
#if defined(WINDOWS)
    return false;
#elif defined(LINUX)
    return (access(filename, F_OK) != -1);
#endif
}

int 
ns_file_open(NsFile *file, char *filename)
{
    file->internal_file = fopen(filename, "rb");
    if(file->internal_file == NULL)
    {
        DebugPrintFileInfo();
        return NS_ERROR;
    }
    return NS_SUCCESS;
}

int
ns_file_open(NsFile *file, const char *filename)
{
    return ns_file_open(file, (char *)filename);
}

int 
ns_file_close(NsFile *file)
{
    if(fclose(file->internal_file) == EOF)
    {
        DebugPrintFileInfo();
        return NS_ERROR;
    }
    return NS_SUCCESS;
}

int 
ns_file_get_size(NsFile *file)
{
    FILE *internal_file = file->internal_file;

    if(fseek(internal_file, 0, SEEK_END) != 0)
    {
        DebugPrintFileInfo();
        return NS_ERROR;
    }

    int filesize = ftell(internal_file);
    if(filesize == -1)
    {
        DebugPrintFileInfo();
        return NS_ERROR;
    }

    rewind(internal_file);

    return filesize;
}

int 
ns_file_load(NsFile *file, char *buffer, int buffer_size)
{
    int filesize = ns_file_get_size(file);
    if(buffer_size < filesize)
    {
        return NS_ERROR;
    }

    int bytes_read = fread(buffer, 1, filesize, file->internal_file);
    if(bytes_read != filesize)
    {
        DebugPrintFileInfo();
        return NS_ERROR;
    }

    return bytes_read;
}
#endif

struct ns_file
{
    uint8_t *Contents;
    int Size;
};

internal ns_file
LoadFile(const char *Filename)
{
    ns_file Result = {0};

    FILE *File = fopen(Filename, "rb");
    Assert(File != NULL);

    int FileSize = 0;
    {
        CheckZero_RR(fseek(File, 0, SEEK_END));
        FileSize = ftell(File);
        CheckNotNeg1_RR(FileSize);
        CheckZero_RR(fseek(File, 0, SEEK_SET));
    }

    uint8_t *FileContents = (uint8_t *)MemAlloc(FileSize);
    size_t BytesRead = fread(FileContents, 1, FileSize, File);
    CheckEquals_RR(BytesRead, (size_t)FileSize);

    CheckZero_RR(fclose(File));

    Result.Contents = FileContents;
    Result.Size = FileSize;
    return Result;
}

int GetFileSize(FILE *File)
{
    int Result = 0;
    CheckZero_RR(fseek(File, 0, SEEK_END));
    Result = ftell(File);
    CheckNotNeg1_RR(Result);
    CheckZero_RR(fseek(File, 0, SEEK_SET));
    return Result;
}

internal void
Free(ns_file *File)
{
    MemFree(File->Contents);
}

#endif
