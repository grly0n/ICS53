// A header file for helpers.c
// Declare any additional functions in this file
#include "icssh.h"
#include "linkedlist.h"
#include <errno.h>

// verifies that argc <= 32 and strlen(argc[i]) <= 63
int invalid_argc_or_length(job_info *job);

// for linked list: compare two bgentry_t instances
int list_comparator(const void *lhs, const void *rhs);

// for linked list: print a bgentry_t instance to fp
void list_printer(void *job, void* fp);

// for linked list: delete a bgengtry_t instance
void list_deleter(void *job);

int removePidFromList(list_t *list, pid_t pid, int bg);

node_t *listContainsPid(list_t *list, pid_t pid);
