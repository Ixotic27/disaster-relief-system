#include "../include/types.h"
#include "../include/fileio.h"
#include "../include/heap.h"
#include "../include/hashmap.h"
#include "../include/graph.h"
#include "../include/allocation.h"
#include "../include/report.h"
#include <stdio.h>
#include <stdlib.h> // Added for free()

int main(int argc, char **argv)
{
    const char *data_dir = "../../data";
    char path_buffer[256]; // Buffer to construct file paths

    Resource *resources = NULL;
    int nres = 0;
    Region *regions = NULL;
    int nreg = 0;
    Request *requests = NULL;
    int nreq = 0;

    snprintf(path_buffer, sizeof(path_buffer), "%s/resources.txt", data_dir);
    if (load_resources(path_buffer, &resources, &nres) != 0)    
    {
        fprintf(stderr, "load_resources failed\n");
        return 1;
    }

    snprintf(path_buffer, sizeof(path_buffer), "%s/regions.txt", data_dir);
    if (load_regions(path_buffer, &regions, &nreg) != 0)
    {
        fprintf(stderr, "load_regions failed\n");
        return 1;
    }

    snprintf(path_buffer, sizeof(path_buffer), "%s/requests.txt", data_dir);
    if (load_requests(path_buffer, &requests, &nreq) != 0)
    {
        fprintf(stderr, "load_requests failed\n");
        return 1;
    }

    HashMap *hm = hm_create(2 * nres + 1);
    for (int i = 0; i < nres; i++)
        hm_put(hm, resources[i].id, &resources[i]);

    Graph *g = graph_create(nreg);
    g->regions = regions;
    g->n = nreg;

    snprintf(path_buffer, sizeof(path_buffer), "%s/edges.txt", data_dir);
    if (load_edges(path_buffer, g) != 0) // Corrected function call
    {
        fprintf(stderr, "load_edges failed\n");
    }

    Heap *h = heap_create(nreq + 5);
    for (int i = 0; i < nreq; i++)
    {
        int sev = region_severity(regions, nreg, requests[i].region_id);
        int priority = sev * 1000 + requests[i].qty_needed;
        heap_insert(h, &requests[i], priority);
    }

    snprintf(path_buffer, sizeof(path_buffer), "%s/report.txt", data_dir);
    init_report(path_buffer);
    
    run_allocator(h, g, hm, regions, nreg);

    hm_free(hm);
    heap_free(h);
    graph_free(g);
    free(resources);
    free(regions);
    free(requests);
    return 0;
}