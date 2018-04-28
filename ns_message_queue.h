#ifndef NS_MESSAGE_BUFFER_H
#define NS_MESSAGE_BUFFER_H

#include "ns_semaphore.h"
#include "ns_common.h"
#include "ns_math.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>


/* Internal */

/* Thread-safe circular buffer. */
struct NsMessageQueue
{
    // NOTE:
    //  -an empty message buffer means (head == tail_
    //  -a full message buffer means (tail + 1 == head)
    //  -we define the end of the buffer to be the beginning. it's easier to just increment 
    //   the head or tail (when adding or removing a message) and just leave it at the end. 

    NsSemaphore read_write_semaphore;

    uint32_t size;
    uint8_t *buffer;
    uint8_t *end;
    uint8_t *head;
    uint8_t *tail;
};


int ns_message_queue_create(NsMessageQueue *message_queue, uint32_t size);
int ns_message_queue_destroy(NsMessageQueue *message_queue);
bool ns_message_queue_check_empty(NsMessageQueue *message_queue);
bool ns_message_queue_check_has_room(NsMessageQueue *message_queue, uint32_t message_size);

void ns_mesage_queue_print(NsMessageQueue *message_queue)
{
    printf("message_queue: start: %p, end: %p, head: %p, tail: %p\n",
           message_queue->buffer, message_queue->end, message_queue->head, message_queue->tail);
}

/* Returns whether there's enough room. We pass the head in case the NsMessageQueue's
   head changes in between the call. */
int ns_message_queue_get_insertion_pointers(NsMessageQueue *message_queue, const uint8_t *head, 
                                            const uint32_t message_size, 
                                            uint8_t **_message_size_ptr, uint8_t **_message_ptr)
{
    uint8_t *message_size_ptr = message_queue->tail;
    {
        // does the size wrap?
        if((message_size_ptr + sizeof(uint32_t)) > message_queue->end)
        {
            // would we go past the head?
            if(message_size_ptr < head)
            {
                DebugPrintInfo();
                return NS_ERROR;
            }

            message_size_ptr = message_queue->buffer;
        }
    }

    uint8_t *message_ptr = (message_size_ptr + sizeof(uint32_t));
    {
        if((message_size_ptr < head) && (message_ptr >= head))
        {
            DebugPrintInfo();
            return NS_ERROR;
        } 

        if(message_ptr == message_queue->end)
        {
            message_ptr = message_queue->buffer;
        }
    }

    // do we have room?
    {
        uint32_t bytes_left = 0;
        {
            if(message_ptr > head)
            {
                // wrap around to beginning
                bytes_left += (message_queue->end - message_ptr);

                bytes_left += (head - message_queue->buffer - 1);
            }
            else
            {
                bytes_left += (head - message_ptr - 1); 
            }
        }

        if(message_size > bytes_left)
        {
            DebugPrintInfo();
            return NS_ERROR;
        }
    }

    if((_message_size_ptr != NULL) && (_message_ptr != NULL))
    {
        *_message_size_ptr = message_size_ptr;
        *_message_ptr = message_ptr;
    }

    return NS_SUCCESS;
}

/* API */

int ns_message_queue_create(NsMessageQueue *message_queue, uint32_t size = Kilobytes(4))
{
    message_queue->size = size;
    message_queue->buffer = (uint8_t *)malloc(size);
    if(message_queue->buffer == NULL)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }
    message_queue->end = (message_queue->buffer + size);
    message_queue->head = message_queue->end;
    message_queue->tail = message_queue->end;

    if(ns_semaphore_create(&message_queue->read_write_semaphore) == NS_ERROR)
    {
        return NS_ERROR;
    }

    return NS_SUCCESS;
}

int ns_message_queue_destroy(NsMessageQueue *message_queue)
{
    free(message_queue->buffer);
    if(ns_semaphore_close(&message_queue->read_write_semaphore) == NS_ERROR)
    {
        return NS_ERROR;
    }
    return NS_SUCCESS;
}

bool ns_message_queue_check_empty(NsMessageQueue *message_queue)
{
    bool result = (message_queue->head == message_queue->tail);
    return result;
}

bool ns_message_queue_check_has_room(NsMessageQueue *message_queue, uint32_t message_size)
{
    bool has_room = ns_message_queue_get_insertion_pointers(message_queue, message_queue->head, 
                                                            message_size, NULL, NULL);
    return has_room;
}

/* Returns number of bytes of message added. */
int ns_message_queue_add(NsMessageQueue *message_queue, uint8_t *message, const uint32_t message_size)
{
    if(message_size == 0)
    {
        return NS_SUCCESS;
    }

    if(message_size > message_queue->size)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    const uint8_t *head = message_queue->head;

    uint8_t *message_size_ptr, *message_ptr;
    if(ns_message_queue_get_insertion_pointers(message_queue, head, 
                                               message_size,
                                               &message_size_ptr, &message_ptr) == NS_ERROR)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    *(uint32_t *)message_size_ptr = message_size;

    // copy payload
    uint8_t *tail = message_ptr;
    {
        uint32_t message_left = message_size;

        if(tail > head)
        {
            uint32_t bytes_to_end = (message_queue->end - tail);
            uint32_t bytes_to_copy = min(message_left, bytes_to_end);
            memcpy(tail, message, bytes_to_copy);
            message += bytes_to_copy;
            message_left -= bytes_to_copy;
            if(message_left > 0)
            {
                tail = message_queue->buffer;
            }
            else
            {
                // we're done!
                tail += message_size;
            }
        }

        if(message_left > 0)
        {
            memcpy(tail, message, message_left);
            tail += message_left;
        }
    }
    message_queue->tail = tail;

    if(ns_semaphore_put(&message_queue->read_write_semaphore) == NS_ERROR)
    {
        DebugPrintInfo();
        return NS_ERROR;
    }

    return message_size;
}

int ns_message_queue_add(NsMessageQueue *message_queue, char *message, const uint32_t message_size)
{
    int result = ns_message_queue_add(message_queue, (uint8_t *)message, message_size);
    return result;
}

int ns_message_queue_get(NsMessageQueue *message_queue, 
                         uint8_t *dest, uint32_t dest_size, 
                         bool is_blocking = true)
{
    int status;

    if(is_blocking)
    {
        status = ns_semaphore_get(&message_queue->read_write_semaphore);
        if(status != NS_SUCCESS)
        {
            DebugPrintInfo();
            return status;
        }
    }
    else 
    {
        status = ns_message_queue_check_empty(message_queue);
        if(status != NS_SUCCESS)
        {
            DebugPrintInfo();
            return NS_ERROR;
        }
    }

    const uint8_t *tail = message_queue->tail;

    uint8_t *message_size_ptr = message_queue->head;
    {
        // does the size wrap?
        if((message_size_ptr + sizeof(uint32_t)) > message_queue->end)
        {
            message_size_ptr = message_queue->buffer;
        }
    }

    uint8_t *message_ptr = (message_size_ptr + sizeof(uint32_t));
    {
        if(message_ptr == message_queue->end)
        {
            message_ptr = message_queue->buffer;
        }
    }

    const uint32_t message_size = *(uint32_t *)message_size_ptr;
    assert(message_size != 0);

    // copy payload
    uint8_t *head = message_ptr;
    {
        uint32_t message_left = message_size;
        uint32_t dest_left = dest_size;

        if(head > tail)
        {
            uint32_t bytes_to_end = (message_queue->end - head);
            uint32_t bytes_to_copy = min(dest_left, min(message_left, bytes_to_end));
            memcpy(dest, head, bytes_to_copy);
            dest += bytes_to_copy;
            dest_left -= bytes_to_copy;
            message_left -= bytes_to_copy;

            // is there still more to copy?
            if((dest_left > 0) &&
               (message_left > 0))
            {
                head = message_queue->buffer;
            }
            else
            {
                // we're done
                head += bytes_to_copy;
            }
        }

        // is dest large enough?
        if(dest_left >= message_left)
        {
            uint32_t bytes_to_copy = min(dest_left, message_left);
            memcpy(dest, head, bytes_to_copy);
            head += bytes_to_copy;
        }
        else
        {
            // where we'll put the shortened message size
            uint8_t *shortened_message_size_ptr = ((head + dest_left) - sizeof(uint32_t));

            // do we have enough room for the size in case we run out of dest?
            if(shortened_message_size_ptr >= message_queue->buffer)
            {
                memcpy(dest, head, dest_left);
                message_left -= dest_left;
                head = shortened_message_size_ptr;
            }
            else
            {
                // note that at this point, the head must be pointed at the beginning of the buffer, meaning none of the
                // message has been read from the start of the buffer. if that wasn't the case, it'd be an issue
                // because, for example, if we were on the third byte, then jumped back to the end, then when we started
                // reading again, we'd reread the first two bytes.

                head = (message_queue->end - sizeof(uint32_t));
            }

            *(uint32_t *)head = message_left;

            // also, we didn't completely remove the message, so increment the semaphore
            status = ns_semaphore_put(&message_queue->read_write_semaphore);
            if(status != NS_SUCCESS)
            {
                DebugPrintInfo();
                return status;
            }
        }
    }
    message_queue->head = head;

    return message_size;
}

int ns_message_queue_get(NsMessageQueue *message_queue, char *dest, uint32_t dest_size, bool is_blocking = true)
{
    int result = ns_message_queue_get(message_queue, (uint8_t *)dest, dest_size, is_blocking);
    return result;
}

#endif
