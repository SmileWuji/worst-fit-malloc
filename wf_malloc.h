/*******************************************************************************
 * This file is part of the project "Worst fit malloc".
 * https://github.com/SmileWuji/worst-fit-malloc
 * Please take a look at the README/documentation!  
 *
 * @Smile Wuji
 ******************************************************************************/
#ifndef __WF_MALLOC_H__
#define __WF_MALLOC_H__ 
#include <string.h>

/**
 * Memory block forward declaration.
 */
typedef struct _memblock MemoryBlock;

/** 
 * Memory allocator. 
 * 
 * The exheap member denotes the function which can extend the heap (eg. sbrk). 
 * It returns the length of newly extended memory, or 0 upon failure.
 */
typedef struct
{
    char *heap;
    size_t (*exheap)(size_t);
    int bound;
    MemoryBlock *freelist;
} Allocator;

void wf_allocator_init(Allocator *allocator, char *heap_pointer, size_t (*extend_heap)(size_t));
void *wf_malloc(Allocator *allocator, size_t size);
void *wf_calloc(Allocator *allocator, size_t num, size_t size);
void *wf_realloc(Allocator *allocator, void *poi, size_t new_size);
void wf_free(Allocator *allocator, void *poi);
void *wf_aligned_alloc(Allocator *allocator, size_t alignment, size_t size);

#endif // __WF_MALLOC_H__
