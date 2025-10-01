#include "../include/allocation.h"
#include "../include/report.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int find_region_index_internal(Region *regions, int n, const char *rid)
{
    for (int i = 0; i < n; i++)
        if (strcmp(regions[i].id, rid) == 0)
            return i;
    return -1;
}
int region_index(Region *regions, int n, const char *rid) { return find_region_index_internal(regions, n, rid); }
int region_severity(Region *regions, int n, const char *rid)
{
    int idx = find_region_index_internal(regions, n, rid);
    if (idx < 0)
        return 0;
    return regions[idx].severity;
}
void run_allocator(Heap *h, Graph *g, HashMap *hm, Region *regions, int nreg)
{
    Request *r;
    while ((r = heap_pop(h)) != NULL)
    {
        Resource *res = hm_get(hm, r->resource_id);
        if (!res)
        {
            log_allocation(r->id, r->region_id, r->resource_id, 0, "", 0, "RESOURCE_NOT_FOUND");
            continue;
        }
        int allocated = 0;
        if (res->quantity <= 0)
        {
            log_allocation(r->id, r->region_id, r->resource_id, 0, "", 0, "OUT_OF_STOCK");
            continue;
        }
        if (res->quantity >= r->qty_needed)
        {
            allocated = r->qty_needed;
            res->quantity -= allocated;
        }
        else
        {
            allocated = res->quantity;
            res->quantity = 0;
        }
        char **path = NULL;
        int plen = 0;
        int dist = 0;
        int rc = graph_shortest_path(g, "DEPOT", r->region_id, &path, &plen, &dist);
        if (rc == 0)
        {
            char route[512];
            route[0] = 0;
            for (int i = 0; i < plen; i++)
            {
                strcat(route, path[i]);
                if (i < plen - 1)
                    strcat(route, "->");
                free(path[i]);
            }
            free(path);
            log_allocation(r->id, r->region_id, r->resource_id, allocated, route, dist, allocated == r->qty_needed ? "FULFILLED" : "PARTIAL");
        }
        else
        {
            log_allocation(r->id, r->region_id, r->resource_id, allocated, "NO_PATH", 0, allocated == 0 ? "FAILED" : "PARTIAL_NO_PATH");
        }
    }
}
