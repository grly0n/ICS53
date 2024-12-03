#include "linkedlist.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern void list_deleter(void *node);

list_t *CreateList(void (*deleter)(void*)) {
    list_t *list = malloc(sizeof(list_t));
    list->length = 0;
    list->head = NULL;
    list->deleter = deleter;
    return list;
}

void InsertAtHead(list_t *list, void *val_ref) {
    if (!list || !val_ref) return;
    if (!list->length) list->head = NULL;
    node_t **head = &(list->head);
    node_t *new_node;
    new_node = malloc(sizeof(node_t));
    new_node->data = val_ref;
    new_node->next = *head;
    *head = new_node;
    list->length++;
}

void RemoveFromHead(list_t *list) {
    node_t **head = &(list->head);
    node_t *next_node = NULL;
    if (!list->length) return;
    next_node = (*head)->next;
    list->length--;
    node_t *temp = *head;
    *head = next_node;
    list_deleter(temp);
}

void DeleteList(list_t *list) {
    while (list->head != NULL) RemoveFromHead(list);
    free(list);
}

