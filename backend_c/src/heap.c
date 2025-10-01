#include "../include/heap.h"
#include <stdlib.h>
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
Heap *heap_create(int cap)
{
    Heap *h = malloc(sizeof(Heap));
    h->arr = malloc(sizeof(Request *) * cap);
    h->prio = malloc(sizeof(int) * cap);
    h->size = 0;
    h->cap = cap;
    return h;
}
static void sift_up(Heap *h, int i)
{
    while (i > 0)
    {
        int p = (i - 1) / 2;
        if (h->prio[p] >= h->prio[i])
            break;
        swap(&h->arr[p], &h->arr[i]);
        swapi(&h->prio[p], &h->prio[i]);
        i = p;
    }
}
static void sift_down(Heap *h, int i)
{
    for (;;)
    {
        int l = 2 * i + 1, r = l + 1, largest = i;
        if (l < h->size && h->prio[l] > h->prio[largest])
            largest = l;
        if (r < h->size && h->prio[r] > h->prio[largest])
            largest = r;
        if (largest == i)
            break;
        swap(&h->arr[i], &h->arr[largest]);
        swapi(&h->prio[i], &h->prio[largest]);
        i = largest;
    }
}
void heap_insert(Heap *h, Request *r, int priority)
{
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
Request *heap_pop(Heap *h)
{
    if (h->size == 0)
        return NULL;
    Request *res = h->arr[0];
    h->size--;
    h->arr[0] = h->arr[h->size];
    h->prio[0] = h->prio[h->size];
    sift_down(h, 0);
    return res;
}
void heap_free(Heap *h)
{
    free(h->arr);
    free(h->prio);
    free(h);
}