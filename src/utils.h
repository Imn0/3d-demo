#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>

#define SCREEN_WIDTH 365
#define SCREEN_HEIGHT 235

#define TARGET_DELAY_MS 16
#define MAPSIZE_R 12
#define MAPSIZE_C 8

#define CELLING_COLOR 0xFF202020
#define FLOOR_COLOR 0xFF303030

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef float f32;
typedef double f64;
typedef size_t usize;
typedef ssize_t isize;

typedef struct _v2_f32 { f32 x; f32 y; } v2;
typedef struct _v2_i32 { i32 x; i32 y; } v2i;

typedef struct Node {
    void* data;
    struct Node* next;
    struct Node* prev;
    void (*dtor)(void*);
} Node;

typedef struct LinkedList {
    Node* head;
    Node* tail;
    Node* itr;
    int size;
} LinkedList;


#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLIP(x, min, max)   (((x) < (min)) ? (min) : \
                            (((x) > (max)) ? (max) : (x)))

LinkedList* ll_create();
void ll_append(LinkedList *list, void *data);
void ll_init_iterator(LinkedList *list);
void ll_free_linked_list(LinkedList *list);
void* ll_itr_get_current(LinkedList *list);
void ll_itr_next(LinkedList *list);
bool ll_itr_has_current(LinkedList *list);

// pops current itr element and advances the iterator
void* ll_itr_pop_current(LinkedList *list);