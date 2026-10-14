// list/list.c
//
// Implementation for linked list.
//
// Todd Perkins

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

/* Allocate a new list on the heap. Returns NULL if allocation fails. */
list_t *list_alloc() {
    list_t *mylist = (list_t *) malloc(sizeof(list_t));
    if (mylist == NULL) return NULL;
    mylist->head = NULL;
    return mylist;
}

/* Free all nodes, then the list itself */
void list_free(list_t *l) {
    if (l == NULL) return;

    node_t *current = l->head;
    while (current != NULL) {
        node_t *next = current->next;
        free(current);
        current = next;
    }
    free(l);
}

/* Print the list */
void list_print(list_t *l) {
    if (l == NULL) {
        printf("NULL\n");
        return;
    }
    char *s = listToString(l);
    if (s == NULL) return;
    printf("%s\n", s);
    free(s);
}

/* Return a heap-allocated string representation of the list.
 * Caller must free() the result. */
char* listToString(list_t *l) {
    if (l == NULL) return NULL;

    /* Each value prints as at most "-2147483648->" (13 chars).
     * Allocate 16 per node plus room for "NULL" and the terminator. */
    size_t size = (size_t)list_length(l) * 16 + 8;
    char *buf = (char *) malloc(size);
    if (buf == NULL) return NULL;

    buf[0] = '\0';
    node_t *curr = l->head;
    while (curr != NULL) {
        size_t used = strlen(buf);
        snprintf(buf + used, size - used, "%d->", curr->value);
        curr = curr->next;
    }
    strcat(buf, "NULL");
    return buf;
}

/* Return a new node from the heap. Returns NULL if allocation fails. */
node_t* getNode(elem value) {
    node_t *mynode = (node_t *) malloc(sizeof(node_t));
    if (mynode == NULL) return NULL;
    mynode->value = value;
    mynode->next = NULL;
    return mynode;
}

/* Return the length of the list */
int list_length(list_t *l) {
    if (l == NULL) return 0;

    int count = 0;
    node_t *current = l->head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

/* Add to back of list */
void list_add_to_back(list_t *l, elem value) {
    if (l == NULL) return;

    node_t *new_node = getNode(value);
    if (new_node == NULL) return;

    /* Empty list */
    if (l->head == NULL) {
        l->head = new_node;
        return;
    }

    /* Traverse to last node */
    node_t *current = l->head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_node;
}

/* Add to front of list */
void list_add_to_front(list_t *l, elem value) {
    if (l == NULL) return;

    node_t *new_node = getNode(value);
    if (new_node == NULL) return;

    new_node->next = l->head;
    l->head = new_node;
}

/* Add at specific index. Starting index is 1. Does nothing if the index
 * is out of range (valid range is 1 to length + 1). */
void list_add_at_index(list_t *l, elem value, int index) {
    if (l == NULL || index < 1) return;

    /* Index 1 = front */
    if (index == 1) {
        list_add_to_front(l, value);
        return;
    }

    node_t *current = l->head;

    /* Walk to the node just before the insertion point */
    for (int i = 1; i < index - 1; i++) {
        if (current == NULL) return;
        current = current->next;
    }

    if (current == NULL) return;

    node_t *new_node = getNode(value);
    if (new_node == NULL) return;

    new_node->next = current->next;
    current->next = new_node;
}

/* Remove from back. Returns the removed element, or -1 if the list is empty. */
elem list_remove_from_back(list_t *l) {
    if (l == NULL || l->head == NULL) return -1;

    /* Only one node */
    if (l->head->next == NULL) {
        elem val = l->head->value;
        free(l->head);
        l->head = NULL;
        return val;
    }

    /* Walk to the second-to-last node */
    node_t *current = l->head;
    while (current->next->next != NULL) {
        current = current->next;
    }

    elem val = current->next->value;
    free(current->next);
    current->next = NULL;
    return val;
}

/* Remove from front. Returns the removed element, or -1 if the list is empty. */
elem list_remove_from_front(list_t *l) {
    if (l == NULL || l->head == NULL) return -1;

    node_t *temp = l->head;
    elem val = temp->value;
    l->head = temp->next;
    free(temp);
    return val;
}

/* Remove at index. Starting index is 1. Returns the removed element,
 * or -1 if the index is out of range. */
elem list_remove_at_index(list_t *l, int index) {
    if (l == NULL || l->head == NULL || index < 1) return -1;

    /* Index 1 = front */
    if (index == 1) {
        return list_remove_from_front(l);
    }

    node_t *current = l->head;

    /* Walk to the node just before the one being removed */
    for (int i = 1; i < index - 1; i++) {
        if (current == NULL) return -1;
        current = current->next;
    }

    if (current == NULL || current->next == NULL) return -1;

    node_t *temp = current->next;
    elem val = temp->value;
    current->next = temp->next;
    free(temp);
    return val;
}

/* Check if element exists in list */
bool list_is_in(list_t *l, elem value) {
    if (l == NULL) return false;

    node_t *current = l->head;
    while (current != NULL) {
        if (current->value == value) return true;
        current = current->next;
    }
    return false;
}

/* Get element at index. Starting index is 1. Returns -1 if the index
 * is out of range. */
elem list_get_elem_at(list_t *l, int index) {
    if (l == NULL || index < 1) return -1;

    node_t *current = l->head;
    for (int i = 1; i < index; i++) {
        if (current == NULL) return -1;
        current = current->next;
    }

    if (current == NULL) return -1;
    return current->value;
}

/* Get index of element. Starting index is 1. Returns -1 if not found. */
int list_get_index_of(list_t *l, elem value) {
    if (l == NULL) return -1;

    node_t *current = l->head;
    int index = 1;
    while (current != NULL) {
        if (current->value == value) return index;
        current = current->next;
        index++;
    }
    return -1;
}
