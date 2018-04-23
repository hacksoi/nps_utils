#ifndef NS_FILES_H
#define NS_FILES_H

#include <stdlib.h>


struct NsFile
{
    FILE *file_pointer;
};


bool open(NpsFile *file, char *filename)
{
    file->file_pointer = fopen(filename, "rb");
    if(file_fp == NULL)
    {
        DEBUG_PRINT_INFO();
        return false;
    }
    return true;
}

bool close(NpsFile *file)
{
    if(fclose(file_fp) == EOF)
    {
        DEBUG_PRINT_INFO();
        return false;
    }
    return true;
}

int get_filesize(NpsFile *file)
{
    FILE *file_pointer = file->file_pointer;

    if(fseek(file_pointer, 0, SEEK_END) != 0)
    {
        DEBUG_PRINT_INFO();
        return -1;
    }

    int filesize = ftell(file_pointer);
    if(filesize == -1)
    {
        DEBUG_PRINT_INFO();
        return -1;
    }

    rewind(file_pointer);

    return filesize;
}

int load(NpsFile *file, char *buffer, uint32_t buffer_size)
{
    int filesize = get_filesize(file);
    if(buffer_size < filesize)
    {
        DEBUG_PRINT_INFO();
        return -1;
    }

    int bytes_read = fread(buffer, 1, filesize, file->file_pointer);
    if(bytes_read != filesize)
    {
        DEBUG_PRINT_INFO();
        return -1;
    }

    return bytes_read;
}

#endif
