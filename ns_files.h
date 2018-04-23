#ifndef NS_FILES_H
#define NS_FILES_H

#include "ns_common.h"

#include <stdlib.h>


struct NsFile
{
    FILE *internal_file;
};


bool ns_file_open(NsFile *file, char *filename)
{
    file->internal_file = fopen(filename, "rb");
    if(file->internal_file == NULL)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }
    return NS_SUCCESS;
}

int ns_file_close(NsFile *file)
{
    if(fclose(file->internal_file) == EOF)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }
    return NS_SUCCESS;
}

int ns_file_get_filesize(NsFile *file)
{
    FILE *internal_file = file->internal_file;

    if(fseek(internal_file, 0, SEEK_END) != 0)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    int filesize = ftell(internal_file);
    if(filesize == -1)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    rewind(internal_file);

    return filesize;
}

int ns_file_load(NsFile *file, char *buffer, int buffer_size)
{
    int filesize = ns_file_get_filesize(file);
    if(buffer_size < filesize)
    {
        return NS_ERROR;
    }

    int bytes_read = fread(buffer, 1, filesize, file->internal_file);
    if(bytes_read != filesize)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    return bytes_read;
}

#endif
