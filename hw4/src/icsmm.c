#include "icsmm.h"
#include "debug.h"
#include "helpers.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * The allocator MUST store the head of its free list in this variable. 
 * Doing so will make it accessible via the extern keyword.
 * This will allow ics_freelist_print to access the value from a different file.
 */
ics_free_header *freelist_head = NULL;

/*
 * The allocator MUST use this pointer to refer to the position in the free list to 
 * starting searching from. 
 */
ics_free_header *freelist_next = NULL;

/*
 * This is your implementation of malloc. It acquires uninitialized memory from  
 * ics_inc_brk() that is 16-byte aligned, as needed.
 *
 * @param size The number of bytes requested to be allocated.
 *
 * @return If successful, the pointer to a valid region of memory of at least the
 * requested size is returned. Otherwise, NULL is returned and errno is set to 
 * ENOMEM - representing failure to allocate space for the request.
 * 
 * If size is 0, then NULL is returned and errno is set to EINVAL - representing
 * an invalid request.
 */
void *prologue = NULL, *epilogue = NULL;
int firstCall = 1, pushBackHeader = 0;
void *ics_malloc(size_t size) { 
    if (!size) {errno = EINVAL; return NULL;}   // If no size, exit with an error
    
    if (!freelist_head) {   // If no free space, allocate a new page
        // Page request
        void *page_start = ics_inc_brk();
        if (page_start == (void*)-1) {errno = ENOMEM; return NULL;}
        // Prologue initialization (only on first call)
        if (firstCall) {
            ((ics_footer*)page_start)->block_size = 0 | 1;
            ((ics_footer*)page_start)->fid = FOOTER_MAGIC;
            prologue = page_start;
            page_start += 8;
        }
        // Epilogue initialization
        void *page_end = ics_get_brk()-8;
        ((ics_header*)page_end)->block_size = 0 | 1;
        ((ics_header*)page_end)->hid = HEADER_MAGIC;
        epilogue = page_end;
        // Free header initialization
        size_t freeBlockSize = 4080;
        // If a call must allocate a new page, push the header back to overwrite the epilogue of the previous page
        // Ensures that the data will be 16-byte aligned and the epilogue will not result in a splinter
        if (!firstCall) {
            page_start -= 8;
            freeBlockSize += 8;
            pushBackHeader = 1;
        }
        ((ics_free_header*)page_start)->prev = NULL;
        ((ics_free_header*)page_start)->next = NULL;
        ((ics_free_header*)page_start)->header.block_size = freeBlockSize;
        ((ics_free_header*)page_start)->header.hid = HEADER_MAGIC;
        freelist_head = (ics_free_header*)page_start;
        freelist_next = (ics_free_header*)page_start;
        // Free footer initialization
        page_end -= 8;
        ((ics_footer*)page_end)->block_size = freeBlockSize;
        ((ics_footer*)page_end)->fid = FOOTER_MAGIC;
        // Page allocation complete; request malloc again
        if (firstCall) firstCall = 0;
        return ics_malloc(size);
    }

    // Align size to 16 bytes if necessary
    size_t aligned_size = 0;
    if (size <= 16) aligned_size = 16;
    else if (size % 16 == 0) aligned_size = size;
    else aligned_size = size + (16 - (size % 16));

    
    // Search through freelist for adequately sized space
    ics_free_header *curr = freelist_next, *start = NULL;
    while (curr != start) {
        if (curr->header.block_size >= aligned_size+16) break;
        if (!start) start = curr;
        if (!curr->next) curr = freelist_head;
        else curr = curr->next;
    }

    // If not enough free space, allocate a new page
    if (curr == start) {
        // New page request
        void *page_start = ics_inc_brk();
        if (page_start == (void*)-1) {errno = ENOMEM; return NULL;}
        // Coalesce with previous free header
        page_start -= 16;
        size_t updatedSize = 4088;
        if (((ics_footer*)page_start)->fid == FOOTER_MAGIC && !(((ics_footer*)page_start)->block_size & 1)) {
            page_start -= ((ics_footer*)page_start)->block_size - 8;
            updatedSize = ((ics_footer*)page_start)->block_size + 4096;
            RemoveFromList(page_start);
            InsertIntoList(page_start, updatedSize);
            if (freelist_head && !freelist_next) freelist_next = page_start;
        } else { // If not coalescing, insert free space as normal
            page_start += 8;
            InsertIntoList(page_start, updatedSize);
        }
        // Update epilogue
        void *page_end = ics_get_brk()-8;
        ((ics_header*)page_end)->hid = HEADER_MAGIC;
        ((ics_header*)page_end)->block_size = 0 | 1;
        epilogue = page_end;
        // Request again
        return ics_malloc(size);
    }
    
    // Allocated header initialization
    ics_header *toAllocate = (ics_header*)RemoveFromList(curr);
    ics_free_header *next_free = ((ics_free_header*)toAllocate)->next; // FOR GRADESCOPE TEST 24
    size_t freeSpaceSize = toAllocate->block_size;
    toAllocate->requested_size = size;
    toAllocate->hid = HEADER_MAGIC;
    toAllocate->block_size = (aligned_size+16) | 1;
    void* allocateStart = (void*)++toAllocate;
    // Splinter handling
    size_t leftover = freeSpaceSize - (aligned_size + 16);
    if (leftover < 32) { // Splintered: increase block_size by size of splinter
        (toAllocate-1)->block_size = (aligned_size+leftover+16) | 1;
        toAllocate = (void*)toAllocate + leftover + aligned_size; 
        ((ics_footer*)toAllocate)->block_size = (aligned_size+leftover+16) | 1;
    } else { // Non-splintered: do not change block_size
        toAllocate = (void*)toAllocate + aligned_size;
        ((ics_footer*)toAllocate)->block_size = (aligned_size+16) | 1;
    }
    // Allocated footer initialization
    ((ics_footer*)toAllocate)->requested_size = size;
    ((ics_footer*)toAllocate)->fid = FOOTER_MAGIC;
    // If splintered, return from malloc
    if (leftover < 32) {
        if (freelist_head && next_free) freelist_next = next_free;
        else if (freelist_head && !freelist_next) freelist_next = freelist_head; // FOR GRADESCOPE TEST 24
        return allocateStart;
    // If not splintered, return free space to freelist
    } else {
        toAllocate++;
        // Special case for when malloc is the first call on a non-first new page: push the free space header back 8 bytes to
        // account for shift in allocated header (in order to eliminate splintering from previous epilogue)
        if (pushBackHeader) {leftover += 8; pushBackHeader = 0;}
        InsertIntoList((ics_free_header*)toAllocate, leftover);
        freelist_next = (ics_free_header*)toAllocate;
        return allocateStart;
    }
}

/*
 * Marks a dynamically allocated block as no longer in use and coalesces with 
 * adjacent free blocks (as specified by Homework Document). 
 * Adds the block to the appropriate bucket according to the block placement policy.
 *
 * @param ptr Address of dynamically allocated memory returned by the function
 * ics_malloc.
 * 
 * @return 0 upon success, -1 if error and set errno accordingly.
 * 
 * If the address of the memory being freed is not valid, this function sets errno
 * to EINVAL. To determine if a ptr is not valid, (i) the header and footer are in
 * the managed  heap space, (ii) check the hid field of the ptr's header for
 * special value (iii) check the fid field of the ptr's footer for special value,
 * (iv) check that the block_size in the ptr's header and footer are equal, (v) 
 * the allocated bit is set in both ptr's header and footer, and (vi) the 
 * requested_size is identical in the header and footer.
 */
int ics_free(void *ptr) {
    // Error if ptr is not in between the prologue and epilogue
    if (ptr < prologue || ptr > epilogue) goto invalidPtr;
    // Error if hid is invalid or header allocated bit is 0
    void *ptr_header = ptr-8;
    if (((ics_header*)ptr_header)->hid != HEADER_MAGIC || !(((ics_header*)ptr_header)->block_size & 1)) goto invalidPtr;
    // Error if fid is invalid or footer allocated bit is 0
    void *ptr_footer = ptr_header + ((ics_header*)ptr_header)->block_size-9;
    if (((ics_footer*)ptr_footer)->fid != FOOTER_MAGIC || !(((ics_footer*)ptr_footer)->block_size & 1)) goto invalidPtr;
    // Error if block_sizes of header and footer are not equal
    if (((ics_header*)ptr_header)->block_size != ((ics_footer*)ptr_footer)->block_size) goto invalidPtr;
    // Error if requested_sizes of header and footer are not equal
    if (((ics_header*)ptr_header)->requested_size != ((ics_footer*)ptr_footer)->requested_size) goto invalidPtr;

    // Check footer of adjacent block
    ptr -= 16;
    // Coalesce if adjacent block is free
    if (((ics_footer*)ptr)->fid == FOOTER_MAGIC && !(((ics_footer*)ptr)->block_size & 1)) {
        ptr_header -= ((ics_footer*)ptr)->block_size;
        ptr += 8;
        size_t newFreeSize = ((ics_header*)ptr_header)->block_size + ((ics_header*)ptr)->block_size-1;
        ((ics_header*)ptr_header)->block_size = newFreeSize;
        ((ics_footer*)ptr_footer)->block_size = newFreeSize;
    // If not coalescing, insert the new free space into the freelist
    } else InsertIntoList(ptr_header, ((ics_header*)ptr_header)->block_size-1);
    return 0;

    invalidPtr:
        errno = EINVAL;
        return -1;
}

/*
 * Resizes the dynamically allocated memory, pointed to by ptr, to at least size 
 * bytes. See Homework Document for specific description.
 *
 * @param ptr Address of the previously allocated memory region.
 * @param size The minimum size to resize the allocated memory to.
 * @return If successful, the pointer to the block of allocated memory is
 * returned. Else, NULL is returned and errno is set appropriately.
 *
 * If there is no memory available ics_malloc will set errno to ENOMEM. 
 *
 * If ics_realloc is called with an invalid pointer, set errno to EINVAL. See ics_free
 * for more details.
 *
 * If ics_realloc is called with a valid pointer and a size of 0, the allocated     
 * block is free'd and return NULL.
 */
void *ics_realloc(void *ptr, size_t size) {
    // Error if ptr is not in between the prologue and epilogue
    if (ptr < prologue || ptr > epilogue) goto invalidPtr;
    // Error if hid is invalid or header allocated bit is 0
    void *ptr_header = ptr-8;
    if (((ics_header*)ptr_header)->hid != HEADER_MAGIC || !(((ics_header*)ptr_header)->block_size & 1)) goto invalidPtr;
    // Error if fid is invalid or footer allocated bit is 0
    void *ptr_footer = ptr_header + ((ics_header*)ptr_header)->block_size-9;
    if (((ics_footer*)ptr_footer)->fid != FOOTER_MAGIC || !(((ics_footer*)ptr_footer)->block_size & 1)) goto invalidPtr;
    // Error if block_sizes of header and footer are not equal
    if (((ics_header*)ptr_header)->block_size != ((ics_footer*)ptr_footer)->block_size) goto invalidPtr;
    // Error if requested_sizes of header and footer are not equal
    if (((ics_header*)ptr_header)->requested_size != ((ics_footer*)ptr_footer)->requested_size) goto invalidPtr;

    // If size == 0, free ptr and return NULL
    if (!size) {ics_free(ptr); return NULL;}

    // Otherwise, allocate space for the new block
    void *new_ptr = ics_malloc(size);
    if (!new_ptr) return NULL;

    // Copy min(payload, size) bytes from the old ptr to the new ptr
    int bytesToCopy = ((ics_header*)ptr_header)->block_size-17, bytesCopied = 0;
    void *new_ptr_cpy = new_ptr, *ptr_cpy = ptr;
    while (bytesCopied < bytesToCopy && bytesCopied < size) {
        *(char*)new_ptr_cpy = *(char*)ptr_cpy;
        new_ptr_cpy++;
        ptr_cpy++;
        bytesCopied++;
    }
    // Free old ptr and return payload of new ptr
    ics_free(ptr);
    return new_ptr;

    invalidPtr:
        errno = EINVAL;
        return NULL;
}
