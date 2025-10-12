#ifndef ALLOCATION_H
#define ALLOCATION_H

#include "types.h"
#include "heap.h"
#include "hashmap.h"
#include "graph.h"

#define MAX_RESOURCES 10

// Resource allocation for a single resource in a region
typedef struct {
    char resource_id[IDLEN];
    char resource_name[NAMELEN];
    int allocated;
    int needed;
    char status[32];  // FULFILLED, PARTIAL, FAILED, OUT_OF_STOCK
} ResourceAllocation;

// Complete allocation result for a region
typedef struct {
    char region_id[IDLEN];
    char region_name[NAMELEN];
    int severity;
    int population;
    int resource_count;
    ResourceAllocation resources[MAX_RESOURCES];
} RegionAllocationResult;

// Route information for resource distribution
typedef struct {
    char source_region_id[IDLEN];
    char source_region_name[NAMELEN];
    char dest_region_id[IDLEN];
    char dest_region_name[NAMELEN];
    char resource_id[IDLEN];
    char resource_name[NAMELEN];
    int quantity;
    int distance;
    char route[512];  // "Haridwar -> Tehri -> Dehradun"
} ResourceRoute;

// Complete allocation report
typedef struct {
    RegionAllocationResult *regions;
    int region_count;
    ResourceRoute *routes;
    int route_count;
} AllocationReport;

// allocator entry point
void run_allocator(Heap *h, Graph *g, HashMap *hm, Region *regions, int nreg, Resource *resources, int nres);

#endif // ALLOCATION_H