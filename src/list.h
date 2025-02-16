// list.h
#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdio.h>

struct list_Node {
    void* data;
    struct list_Node* next;
};

struct list_Node* listlib_createNode(void* data);
void list_append(struct list_Node** head_ref, void* data);
void list_deleteNode(struct list_Node** head_ref, void* key);
void list_deleteFirstNode(struct list_Node** head_ref);
void list_deleteAtIndex(struct list_Node** head_ref, int index);
int list_findIndex(struct list_Node* head, void* key);
void list_printIntList(struct list_Node* head);
void list_printFloatList(struct list_Node* head);

struct list_SomeStruct {
    int id;
    char* name;
};

void list_printSomeStructList(struct list_Node* head);

#endif  // LISTLIB_H