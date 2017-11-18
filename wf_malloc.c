/*******************************************************************************
 * This file is part of the project "Worst fit malloc".
 * https://github.com/SmileWuji/worst-fit-malloc
 * Please take a look at the README/documentation! 
 *
 * @Smile Wuji
 ******************************************************************************/
#include "wf_malloc.h"

#include <string.h>

/* Macro for referencing the block from an allocated address. */
#define REF_BLOCK(_p) ((MemoryBlock *) (((char *)(_p)) - sizeof(MemoryBlock)))

/* Helper functions for the freelist API. */
MemoryBlock *wf_exheap_available_block(Allocator *allocator, size_t size);
void wf_freelist_offer(Allocator *allocator, MemoryBlock *block);
MemoryBlock *wf_freelist_poll(Allocator *allocator);
void wf_freelist_remove(Allocator *allocator, MemoryBlock *block);

/* Macros for block data. */
#define REF_BLOCK_DATA(_b) (&(_b)->data)
#define REF_BLOCK_DATA_SIZE(_b) ((_b)->size)
#define IS_BLOCK_AVAILABLE(_b) (!(_b)->available.prev && !(_b)->available.next)
#define BLOCK_MARK_UNAVAILABLE(_b) ({                                          \
    (_b)->available.prev = NULL;                                               \
    (_b)->available.next = NULL;                                               \
})

/* Block helper functions. */
MemoryBlock *block_split(MemoryBlock *block, size_t size);
MemoryBlock *block_merge(MemoryBlock *block);
int block_consumable(MemoryBlock *block, size_t new_size);
void block_consume(MemoryBlock *block, size_t new_size);

/** 
 * Memory block definition. 
 * The adjacent member denotes a linked list for adjacent memory blocks.
 * The available member denotes a linked list for implementing the free list.
 * If available.prev and available.next are both NULL, then this memory block is
 * not available.
 */
struct _memblock 
{
    struct {
        struct _memblock *prev;
        struct _memblock *next;
    } adjacent;

    struct {
        struct _memblock *prev;
        struct _memblock *next;
    } available;

    size_t size;  /* Size of data. */

    char data[];
};


/**
 * Initializes an allocator object. 
 */
void wf_allocator_init(
    Allocator *allocator, 
    char *heap_pointer, 
    size_t (*extend_heap)(size_t))
{
    allocator->heap = heap_pointer;
    allocator->exheap = extend_heap;
    allocator->bound = 0;
    allocator->freelist = NULL;
}


/**
 * Allocates size bytes of uninitialized storage.
 */
void *wf_malloc(Allocator *allocator, size_t size)
{
    MemoryBlock *target;
    MemoryBlock *residual;

    if (!allocator->freelist || REF_BLOCK_DATA_SIZE(allocator->freelist) < size)
    {
        target = wf_exheap_available_block(allocator, size);
        if (!target)
        {
            return NULL;
        }
        residual = block_split(target, size);
    }
    else
    {
        target = wf_freelist_poll(allocator);
        residual = block_split(target, size);
        if (!residual)
        {
            block_merge(residual);
            wf_freelist_offer(allocator, target);
            target = wf_exheap_available_block(allocator, size);
            if (!target)
            {
                return NULL;
            }
            residual = block_split(target, size);
        }
    }

    /* Wipe out freelist information. */
    BLOCK_MARK_UNAVAILABLE(target);

    wf_freelist_offer(allocator, residual);
    return REF_BLOCK_DATA(target);
}


/** 
 * Allocates memory for an array of num objects of size size and initializes all
 * bytes in the allocated storage to zero.
 */
void *wf_calloc(Allocator *allocator, size_t num, size_t size)
{
    void *poi;
    size_t bytes;

    bytes = num * size;
    poi = wf_malloc(allocator, bytes);
    if (!poi)
    {
        return NULL;
    }

    return memset(poi, 0, bytes);
}


/**
 * Reallocates the given area of memory. It must be previously allocated by 
 * malloc(), calloc() or realloc() and not yet freed with a call to free or 
 * realloc. Otherwise, the results are undefined.
 */
void *wf_realloc(Allocator *allocator, void *poi, size_t new_size)
{
    MemoryBlock prev_hard_copy;
    MemoryBlock *target;
    MemoryBlock *poimeta;
    size_t old_size;
    void *new_poi;

    if (!poi)
    {
        return wf_malloc(allocator, new_size);
    }

    poimeta = REF_BLOCK(poi);
    old_size = poimeta->size;

    /* Attempts to extend the poimeta block. */
    if (block_consumable(poimeta, new_size))
    {
        block_consume(poimeta, new_size);
        return poi;
    }

    /* The attempt failed. Try to do an undo-able free and move memory. */
    if (poimeta->adjacent.next && IS_BLOCK_AVAILABLE(poimeta->adjacent.next))
    {
        wf_freelist_remove(allocator, poimeta->adjacent.next);
        block_merge(poimeta->adjacent.next);
    }

    /* Copy the previous memory block metadata for being undo-able.*/
    if (!poimeta->adjacent.prev)
    {
        memcpy(&prev_hard_copy, poimeta->adjacent.prev, sizeof(MemoryBlock));
    }

    target = block_merge(poimeta);
    if (target != poimeta)
    {
        /* Update the status of the previous block (if poimeta merged into). */
        wf_freelist_remove(allocator, target);
    }
    wf_freelist_offer(allocator, target);

    new_poi = wf_malloc(allocator, new_size);
    if (!new_poi)
    {
        /* Perform undo... */
        wf_freelist_remove(allocator, target);
        if (target != poimeta)
        {
            /* The poimeta was merged to adjacent.prev. */
            memcpy(target, &prev_hard_copy, sizeof(MemoryBlock));
            wf_freelist_offer(allocator, target);
        }

        if (poimeta->size != old_size)
        {
            /* The next block of the poimeta was merged previously. */
            wf_freelist_offer(allocator, block_split(poimeta, old_size));
        }

        BLOCK_MARK_UNAVAILABLE(poimeta);
        
        return NULL;
    }

    return memmove(new_poi, poi, old_size);
}


/** 
 * Deallocates the space previously allocated by malloc(), calloc(), 
 * aligned_alloc, (since C11) or realloc().
 */
void wf_free(Allocator *allocator, void *poi)
{
    // TODO
}


/**
 * Allocate size bytes of uninitialized storage whose alignment is specified by 
 * alignment. The size parameter must be an integral multiple of alignment.
 */
void *wf_aligned_alloc(Allocator *allocator, size_t alignment, size_t size)
{
    // TODO
    return NULL;
}

/**
 * Calls exheap to the allocator and returns the available block that can hold
 * data having size bytes.
 */
MemoryBlock *wf_exheap_available_block(Allocator *allocator, size_t size)
{
    // TODO
    return NULL;
}


/**
 * Adds a block to the freelist.
 */
void wf_freelist_offer(Allocator *allocator, MemoryBlock *block)
{
    // TODO
}


/**
 * Removes the largest block from the freelist.
 */
MemoryBlock *wf_freelist_poll(Allocator *allocator)
{
    // TODO
    return NULL;
}


/**
 * Removes a specific block from the freelist.
 */
void wf_freelist_remove(Allocator *allocator, MemoryBlock *block)
{
    // TODO
}


/**
 * Splits the block and adjusts the fields in block.
 * Returns the block being splitted from block.
 */
MemoryBlock *block_split(MemoryBlock *block, size_t offset)
{
    // TODO
    return NULL;
}


/**
 * Merges the block into (possibly) its previous adjacent block.
 * If the previous adjacent block is not available, then the block is merged to
 * itself.
 * Adjusts the size of the block being merged into accordingly.
 */
MemoryBlock *block_merge(MemoryBlock *block)
{
    // TODO
    return NULL;
}


/**
 * Returns whether the following adjacent blocks can be consumed by the block.
 * The consumed blocks would have (at least) new_size as its size of data.
 */
int block_consumable(MemoryBlock *block, size_t new_size)
{
    return 0;
}


/**
 * Assuming block_consumable returns nonzero.
 * Consumes the following adjacent blocks.
 * The consumed blocks would have (at least) new_size as its size of data.
 */
void block_consume(MemoryBlock *block, size_t new_size)
{
    // TODO
}
