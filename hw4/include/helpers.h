#ifndef HELPERS_H
#define HELPERS_H
#include "icsmm.h"

// Removes and returns free space from list
ics_free_header *RemoveFromList(ics_free_header *space);

// Inserts free space into list
void InsertIntoList(ics_free_header *space, size_t newFreeSize);


#endif
