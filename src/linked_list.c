#include "utils.h"

LinkedList* ll_create() {
    LinkedList *list = (LinkedList*)malloc(sizeof(LinkedList));
    if (list == NULL) {
        exit(1);
    }
    list->head = NULL;
    list->tail = NULL;
    list->itr = NULL;
    list->size = 0;
    return list;
}

void ll_append(LinkedList *list, void *data) {
    Node *newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        exit(1);
    }
    newNode->data = data;
    newNode->next = NULL;
    if (list->head == NULL) {
        list->head = newNode;
        list->tail = newNode;
    } else {
        list->tail->next = newNode;
        list->tail = newNode;
    }
    list->size++;
}

void ll_init_iterator(LinkedList *list) {
    list->itr = list->head;
}

void* ll_itr_get_current(LinkedList *list) {
    if (list->itr != NULL)
        return list->itr->data;
    else
        return NULL;
}

void ll_itr_next(LinkedList *list) {
    if (list->itr != NULL)
        list->itr = list->itr->next;
}

bool ll_itr_has_current(LinkedList *list) {
    if (list == NULL || list->itr == NULL) {
        return false;
    }
    return list->itr->data != NULL;
}

void* ll_itr_pop_current(LinkedList *list) {
    if (list->itr == NULL) return NULL;
    Node *current = list->itr;
    if (current == list->head) {
        list->head = current->next;
    } else {
        Node *prev = list->head;
        while (prev->next != current) {
            prev = prev->next;
        }
        prev->next = current->next;
        if (current == list->tail)
            list->tail = prev;
    }
    void* data = current->data;
    list->itr = current->next;
    free(current);
    list->size--;
    return data;
}

void ll_free_linked_list(LinkedList *list) {
    Node *current = list->head;
    while (current != NULL) {
        Node *temp = current;
        current = current->next;
        free(temp);
    }
    free(list);
}
