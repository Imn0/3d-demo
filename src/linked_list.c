#include "utils.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

LinkedList* ll_init() {
    i32 result = 0;

    LinkedList* new_list = (LinkedList*)calloc(1, sizeof(LinkedList));
    if (new_list == NULL) {
        result = 1;
        goto fail;
    }

    new_list->head = NULL;
    new_list->tail = NULL;
    new_list->is_circle = false;

    return new_list;

fail:
    ll_destroy(new_list);
    return NULL;
}

i32 ll_push_front(LinkedList* list, void* data) {
    return ll_push_front_dtor(list, data, NULL);
}

i32 ll_push_front_dtor(LinkedList* list, void* data, void (*dtor)(void*)) {
    i32 result = 0;

    Node* new_node = (Node*)calloc(1, sizeof(Node));
    if (new_node == NULL) {
        result = 1;
        goto fail;
    }

    new_node->data = data;
    new_node->next = list->head;
    new_node->prev = NULL;
    new_node->dtor = dtor;

    list->head = new_node;
    if (list->tail == NULL) {
        list->tail = new_node;
    }

    return result;
fail:
    free(new_node);
    return result;
}

i32 ll_push_back(LinkedList* list, void* data) {
    return ll_push_back_dtor(list, data, NULL);
}

i32 ll_push_back_dtor(LinkedList* list, void* data, void (*dtor)(void*)) {
    i32 result = 0;

    Node* new_node = (Node*)calloc(1, sizeof(Node));
    if (new_node == NULL) {
        result = 1;
        goto fail;
    }

    new_node->data = data;
    new_node->next = NULL;
    new_node->dtor = dtor;

    if (list->tail != NULL) {
        new_node->prev = list->tail;
        list->tail->next = new_node;
    }
    list->tail = new_node;
    if (list->head == NULL) {
        list->head = new_node;
    }

    return result;
fail:
    free(new_node);
    return result;
}

void* ll_pop_front(LinkedList* list) {
    if (list->head == NULL) {
        return NULL;
    }

    Node* old_head = list->head;
    void* data = old_head->data;
    list->head = old_head->next;
    list->head->prev = NULL;
    if (list->head == NULL) {
        list->tail = NULL;
    }

    free(old_head);
    return data;

}

void* ll_pop_back(LinkedList* list) {
    if (list->tail == NULL) {
        return NULL;
    }

    Node* old_tail = list->tail;
    void* data = old_tail->data;
    if (list->head == list->tail) {
        list->head = NULL;
        list->tail = NULL;
    }
    else {
        Node* new_tail = list->head;
        while (new_tail->next != list->tail) {
            new_tail = new_tail->next;
        }
        new_tail->next = NULL;
        list->tail = new_tail;
    }

    free(old_tail);
    return data;

}

void* ll_get(LinkedList* list, usize index) {
    Node* current = list->head;
    usize i = 0;
    while (current != NULL) {
        if (i == index) {
            return current->data;
        }
        current = current->next;
        i++;
    }
    return NULL;

}

void* ll_remove(LinkedList* list, usize index) {
    Node* current = list->head;
    usize i = 0;
    while (current != NULL) {
        if (i == index) {
            if (current == list->head) {
                return ll_pop_front(list);
            }
            else if (current == list->tail) {
                return ll_pop_back(list);
            }
            else {
                Node* prev = list->head;
                while (prev->next != current) {
                    prev = prev->next;
                }
                prev->next = current->next;
                void* data = current->data;
                free(current);
                return data;
            }
        }
        current = current->next;
        i++;
    }
    return NULL;

}

i32 ll_print(LinkedList* list, void (*printFunc)(void*)) {
    Node* current = list->head;
    while (current != NULL) {
        printFunc(current->data);
        current = current->next;
    }
    return 0;
}

i32 ll_destroy(LinkedList* list) {
    Node* current = list->head;
    while (current != NULL) {
        Node* next = current->next;
        if (current->dtor != NULL) {
            current->dtor(current->data);
        }
        free(current);
        current = next;
    }
    list->head = NULL;
    list->tail = NULL;
    list->is_circle = false;
    return 0;

}

i32 ll_insert(LinkedList* list, void* data, usize index) {
    i32 result = 0;

    Node* current = list->head;
    usize i = 0;
    while (current != NULL) {
        if (i == index) {
            Node* new_node = (Node*)calloc(1, sizeof(Node));
            if (new_node == NULL) {
                result = 1;
                return result;
            }

            new_node->data = data;
            new_node->next = current->next;
            current->next = new_node;
            return result;
        }
        current = current->next;
        i++;
    }

    return result;

}

i32 ll_make_circle(LinkedList* list) {
    list->is_circle = true;
    return 0;

}
i32 ll_remove_cirlce(LinkedList* list) {
    list->is_circle = false;
    return 0;
}

ListItr* ll_itr_assign(LinkedList* list) {
    assert(list != NULL);

    ListItr* new_itr = (ListItr*)calloc(1, sizeof(ListItr));
    if (new_itr == NULL) {
        return 1;
    }
    new_itr->list = list;
    new_itr->current = list->head;

    return new_itr;
}

i32 ll_itr_next(ListItr* itr) {
    assert(itr != NULL);

    itr->current = itr->current->next;
    if (itr->current == NULL && itr->list->is_circle) {
        itr->current = itr->list->head;
    }
    return 0;
}

void* ll_itr_get(ListItr* itr) {
    assert(itr != NULL);
    if(itr->current != NULL){
        printf("getting %p\n", itr->current);
        return itr->current->data;
    }
    return NULL;
}

void* ll_itr_pop_current(ListItr* itr) {
    assert(itr != NULL);
    assert(itr->current != NULL);

    void* data = itr->current->data;
    Node* node_to_remove = itr->current;

    if (node_to_remove == itr->list->head) {
        itr->list->head = itr->current->next;
        if (itr->list->head != NULL) {
            itr->list->head->prev = NULL;
        }
    } else {
        node_to_remove->prev->next = node_to_remove->next;
    }

    if (node_to_remove == itr->list->tail) {
        itr->list->tail = itr->current->prev;
        if (itr->list->tail != NULL) {
            itr->list->tail->next = NULL;
        }
    } else {
        node_to_remove->next->prev = node_to_remove->prev;
    }
    free(node_to_remove);
    return data;
}

bool ll_itr_has_next(ListItr* itr) {
    assert(itr != NULL);
    return itr->current != NULL;
}
