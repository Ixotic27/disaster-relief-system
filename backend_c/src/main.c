#include "../include/types.h"
#include "../include/fileio.h"
#include "../include/heap.h"
#include "../include/hashmap.h"
#include "../include/graph.h"
#include "../include/allocation.h"
#include "../include/report.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    const char *dataPath = "../../data";
    char filePath[256];

    Resource *resources = NULL;
    Region *regions = NULL;
    Request *requests = NULL;
    int nres = 0, nreg = 0, nreq = 0;

    // === Load Resources ===
    snprintf(filePath, sizeof(filePath), "%s/resources.txt", dataPath); // using snprintf to avoid buffer overflow as it will safely print the data in the file
    if (load_resources(filePath, &resources, &nres) != 0)
    {
        fprintf(stderr, "Error: Failed to load resources\n");
        return 1;
    }

    // === Load Regions ===
    snprintf(filePath, sizeof(filePath), "%s/regions.txt", dataPath);
    if (load_regions(filePath, &regions, &nreg) != 0)
    {
        fprintf(stderr, "Error: Failed to load regions\n");
        return 1;
    }

    // === Load Requests ===
    snprintf(filePath, sizeof(filePath), "%s/requests.txt", dataPath);
    if (load_requests(filePath, &requests, &nreq) != 0)
    {
        fprintf(stderr, "Error: Failed to load requests\n");
        return 1;
    }

    // === Setup HashMap for Resources ===
    HashMap *hm = hm_create(2 * nres + 1);
    for (int i = 0; i < nres; i++)
        hm_put(hm, resources[i].id, &resources[i]);

    // === Setup Graph for Regions ===
    Graph *g = graph_create(nreg);
    g->regions = regions;
    g->n = nreg;

    snprintf(filePath, sizeof(filePath), "%s/edges.txt", dataPath);
    if (load_edges(filePath, g) != 0)
        fprintf(stderr, "Warning: Failed to load edges\n");

    // === Setup Heap for Requests ===
    Heap *h = heap_create(nreq + 5);
    for (int i = 0; i < nreq; i++)
    {
        int severity = region_severity(regions, nreg, requests[i].region_id);
        int priority = severity * 1000 + requests[i].qty_needed;
        heap_insert(h, &requests[i], priority);
    }

    // === Generate Report ===
    snprintf(filePath, sizeof(filePath), "%s/report.txt", dataPath);
    init_report(filePath);

    // === Allocation Process ===
    run_allocator(h, g, hm, regions, nreg);

    // === Cleanup ===
    hm_free(hm);
    heap_free(h);
    graph_free(g);
    free(resources);
    free(regions);
    free(requests);

    return 0;
}
