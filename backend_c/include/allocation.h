#ifndef ALLOCATION_H
#define ALLOCATION_H
#include "types.h"
#include "graph.h"
#include "hashmap.h"
#include "heap.h"
void run_allocator(Heap *h, Graph *g, HashMap *hm, Region *regions, int nreg);
int region_severity(Region *regions, int n, const char *rid);
int region_index(Region *regions, int n, const char *rid);
#endif
