#include "../include/heap.h"
#include <stdlib.h>

// ===== Swap helpers =====
static void swap(Request **a, Request **b)
{
    Request *t = *a;
    *a = *b;
    *b = t;
}

static void swapi(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

// ===== Create a heap =====
Heap *heap_create(int cap)
{
    Heap *h = malloc(sizeof(Heap));
    h->arr = malloc(sizeof(Request *) * cap); // array of pointers to requests
    h->prio = malloc(sizeof(int) * cap);      // array of priorities
    h->size = 0;
    h->cap = cap;
    return h;
}

// ===== Maintain heap property upwards =====
static void sift_up(Heap *h, int i)
{
    while (i > 0)
    {
        int p = (i - 1) / 2; // parent index
        if (h->prio[p] >= h->prio[i]) break;
        swap(&h->arr[p], &h->arr[i]);
        swapi(&h->prio[p], &h->prio[i]);
        i = p;
    }
}

// ===== Maintain heap property downwards =====
static void sift_down(Heap *h, int i)
{
    for (;;)
    {
        int l = 2 * i + 1, r = l + 1, largest = i;
        if (l < h->size && h->prio[l] > h->prio[largest]) largest = l;
        if (r < h->size && h->prio[r] > h->prio[largest]) largest = r;
        if (largest == i) break;
        swap(&h->arr[i], &h->arr[largest]);
        swapi(&h->prio[i], &h->prio[largest]);
        i = largest;
    }
}

// ===== Insert a request with priority =====
void heap_insert(Heap *h, Request *r, int priority)
{
    // Resize if full
    if (h->size >= h->cap)
    {
        h->cap *= 2;
        h->arr = realloc(h->arr, sizeof(Request *) * h->cap);
        h->prio = realloc(h->prio, sizeof(int) * h->cap);
    }

    h->arr[h->size] = r;
    h->prio[h->size] = priority;
    sift_up(h, h->size);
    h->size++;
}

// ===== Pop the highest priority request =====
Request *heap_pop(Heap *h)
{
    if (h->size == 0) return NULL;

    Request *res = h->arr[0];
    h->size--;

    h->arr[0] = h->arr[h->size];
    h->prio[0] = h->prio[h->size];
    sift_down(h, 0);

    return res;
}

// ===== Free heap memory =====
void heap_free(Heap *h)
{
    free(h->arr);
    free(h->prio);
    free(h);
}
