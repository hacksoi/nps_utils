#ifndef NS_FILES_H
#define NS_FILES_H

#include "ns_common.h"

#if defined(WINDOWS)
#elif defined(LINUX)
    #include <unistd.h>
#endif

#include <stdlib.h>


#if defined(WINDOWS)
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
