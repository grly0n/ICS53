#include "helpers.h"
#include "debug.h"

extern ics_free_header *freelist_head;
extern ics_free_header *freelist_next;


// Removes and returns free space from list
ics_free_header *RemoveFromList(ics_free_header *space) {
    ics_free_header *curr = freelist_head;
    while (curr) {
        if (curr->next == space->next) break;
        else curr = curr->next;
    }
    if (!curr) return NULL;
    if (curr->prev) curr->prev->next = curr->next;
    else freelist_head = curr->next;
    if (curr->next) curr->next->prev = curr->prev;
    if (freelist_head == curr) freelist_head = NULL;
    if (freelist_next == curr) freelist_next = NULL;
    return curr;
}


// Inserts a free space into list
void InsertIntoList(ics_free_header *space, size_t newFreeSize) {
    space->header.hid = HEADER_MAGIC;
    space->header.block_size = newFreeSize;
    ics_footer *space_footer = (void*)space + newFreeSize - 8;
    space_footer->block_size = newFreeSize;
    space_footer->fid = FOOTER_MAGIC;
    ics_free_header *curr = freelist_head, *prev_curr = NULL;
    if (!freelist_head) {   // Reinsert at head if only free space in heap
        space->prev = NULL;
        space->next = NULL;
        freelist_head = space;
        freelist_next = space;
    } else {
        while ((void*)curr < (void*)space && curr) {prev_curr = curr; curr = curr->next;}
        if (!curr) {            // Inserted at end of list
            prev_curr->next = space;
            space->prev = prev_curr;
            space->next = NULL;
        }
        else if (!prev_curr) {  // Inserted at head of list
            curr->prev = space;
            space->next = curr;
            space->prev = NULL;
            freelist_head = space;
        } else {                // Inserted in middle of list
            prev_curr->next = space;
            space->prev = prev_curr;
            curr->prev = space;
            space->next = curr;
        }
    }
}
