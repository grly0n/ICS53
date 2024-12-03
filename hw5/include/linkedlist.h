#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

typedef struct node {
    void *data;
    struct node *next;
} node_t;

typedef struct list {
    node_t *head;
    int length;
    void (*deleter)(void*);
} list_t;

list_t *CreateList(); 
void InsertAtHead(list_t *list, void *val_ref); 
void RemoveFromHead(list_t *list); 
void DeleteList(list_t *list); 
void SearchAndAddUsername(list_t *list, char *username);

#endif
