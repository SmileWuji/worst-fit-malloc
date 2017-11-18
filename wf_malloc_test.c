/*******************************************************************************
 * This file is part of the project "Worst fit malloc".
 * https://github.com/SmileWuji/worst-fit-malloc
 * Please take a look at the README/documentation! 
 *
 * @Smile Wuji
 ******************************************************************************/
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "wf_malloc.h"

#define PAGE_SIZE 4096

size_t extend_heap(size_t size);
char *get_heap();

/**
 * Physical memory array.
 */
char physmem[65536];
size_t physmem_length = 65536;
size_t physmem_size = 0;


/**
 * Extends the heap by (at least) size. 
 * Returns the length of newly extended memory, or 0 upon failure.
 */
size_t extend_heap(size_t size)
{
    size_t actual;
    size_t aligned;
    size_t unaligned;

    if (size < 0)
    {
        return 0;
    }

    for (int i = 0; size < actual; ++i)
    {
        actual = PAGE_SIZE * i;
    }

    aligned = physmem_size + actual;
    unaligned = physmem_size + size;
    if (aligned > physmem_length)
    {
        if (unaligned > physmem_length)
        {
            errno = ENOMEM;
            return 0;
        }

        actual = size;
    }
    
    physmem_size += actual;

    return actual;
}


char *get_heap()
{
    return physmem;
}


int main(int argc, char const *argv[])
{
    printf("UNFINISHED... \n");
    return 0;
}
