#ifndef HEAP_H
#define HEAP_H

#include "types.h"

typedef struct {
    Request **arr;
    int *prio;
    int size;
    int cap;
} Heap;

Heap *heap_create(int cap);
void heap_insert(Heap *h, Request *r, int priority);
Request *heap_pop(Heap *h);
void heap_free(Heap *h);

#endif // HEAP_H
