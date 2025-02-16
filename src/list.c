// list.c
#include "list.h"

struct list_Node* list_createNode(void* data) {
    struct list_Node* node = (struct list_Node*)malloc(sizeof(struct list_Node));
    if (node == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    node->data = data;
    node->next = NULL;
    return node;
}

void list_append(struct list_Node** head_ref, void* data) {
    struct list_Node* new_node = list_createNode(data);
    struct list_Node* last = *head_ref;
    if (*head_ref == NULL) {
        *head_ref = new_node;
        return;
    }
    while (last->next != NULL) {
        last = last->next;
    }
    last->next = new_node;
}

void list_deleteNode(struct list_Node** head_ref, void* key) {
    struct list_Node* temp = *head_ref;
    struct list_Node* prev = NULL;
    if (temp != NULL && temp->data == key) {
        *head_ref = temp->next;
        free(temp);
        return;
    }
    while (temp != NULL && temp->data != key) {
        prev = temp;
        temp = temp->next;
    }
    if (temp == NULL) {
        return;
    }
    prev->next = temp->next;
    free(temp);
}

void list_deleteFirstNode(struct list_Node** head_ref) {
    struct list_Node* temp = *head_ref;
    if (temp == NULL) {
        return;
    }
    *head_ref = temp->next;
    free(temp);
}

void list_deleteAtIndex(struct list_Node** head_ref, int index) {
    struct list_Node* temp = *head_ref;
    struct list_Node* prev = NULL;
    if (index == 0 && temp != NULL) {
        *head_ref = temp->next;
        free(temp);
        return;
    }
    int i = 0;
    while (temp != NULL && i != index) {
        prev = temp;
        temp = temp->next;
        i++;
    }
    if (temp == NULL) {
        return;
    }
    prev->next = temp->next;
    free(temp);
}

int list_findIndex(struct list_Node* head, void* key) {
    struct list_Node* temp = head;
    int index = 0;
    while (temp != NULL) {
        if (temp->data == key) {
            printf("Index: %d\n", index);
            return index;
        }
        temp = temp->next;
        index++;
    }
    printf("Element not found\n");
    return -1;
}

void list_printIntList(struct list_Node* head) {
    struct list_Node* temp = head;
    while (temp != NULL) {
        printf("%d -> ", *(int*)(temp->data));
        temp = temp->next;
    }
    printf("NULL\n");
}

void list_printFloatList(struct list_Node* head) {
    struct list_Node* temp = head;
    while (temp != NULL) {
        printf("%.2f -> ", *(float*)(temp->data));
        temp = temp->next;
    }
    printf("NULL\n");
}