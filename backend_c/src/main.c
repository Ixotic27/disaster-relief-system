#include "../include/types.h"
#include "../include/fileio.h"
#include "../include/heap.h"
#include "../include/hashmap.h"
#include "../include/graph.h"
#include "../include/allocation.h"
#include "../include/report.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    const char *dataPath = "../data";
    char filePath[256];

    Resource *resources = NULL;
    Region *regions = NULL;
    int nres = 0, nreg = 0;

    printf("\n=== Disaster Resource Allocation System (Non-Interactive Mode) ===\n\n");

    //Load Resource
    snprintf(filePath, sizeof(filePath), "%s/resources.txt", dataPath);
    if (load_resources(filePath, &resources, &nres) != 0)
    {
        fprintf(stderr, "Error: Failed to load resources from %s\n", filePath);
        return 1;
    }
    printf("✓ Loaded %d resources\n", nres);

    //Load Regions
    snprintf(filePath, sizeof(filePath), "%s/regions.txt", dataPath);
    if (load_regions(filePath, &regions, &nreg) != 0)
    {
        fprintf(stderr, "Error: Failed to load regions from %s\n", filePath);
        return 1;
    }
    printf("✓ Loaded %d regions\n", nreg);

    //Setup HashMap for Resources
    HashMap *hm = hm_create(2 * nres + 1);
    for (int i = 0; i < nres; i++)
        hm_put(hm, resources[i].id, &resources[i]);

    //Setup Graph for Regions
    Graph *g = graph_create(nreg);
    g->regions = regions;
    g->n = nreg;

    snprintf(filePath, sizeof(filePath), "%s/edges.txt", dataPath);
    if (load_edges(filePath, g) != 0)
    {
        fprintf(stderr, "Warning: Failed to load edges from %s\n", filePath);
    }
    else
    {
        printf("✓ Loaded edges\n");
    }

    // Identify disaster and safe regions from severity field
    int disasterCount = 0;
    int *is_disaster = calloc(nreg, sizeof(int));
    if (!is_disaster)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 1;
    }

    printf("\nRegion Status:\n");
    for (int i = 0; i < nreg; i++)
    {
        if (regions[i].severity > 0)
        {
            is_disaster[i] = 1;
            disasterCount++;
            printf("  [DISASTER] %s (severity: %d, population: %d)\n", 
                   regions[i].name, regions[i].severity, regions[i].population);
        }
        else
        {
            is_disaster[i] = 0;
            printf("  [SAFE] %s (population: %d)\n", 
                   regions[i].name, regions[i].population);
        }
    }

    if (disasterCount == 0)
    {
        fprintf(stderr, "Error: No disaster regions found (severity > 0)\n");
        free(is_disaster);
        free(resources);
        free(regions);
        return 1;
    }

    printf("\nFound %d disaster region(s) and %d safe region(s)\n", 
           disasterCount, nreg - disasterCount);

    //Load per-region resource stocks
    // Resources will be loaded from region_resources.txt by allocation.c
    printf("\n✓ Per-region resource tracking enabled\n");
    printf("  (Resources will be loaded from region_resources.txt)\n");

    //Prepare Heap of Disaster Requests
    // Create a request for EACH resource in each affected region
    Heap *h = heap_create(disasterCount * nres + 5);
    int req_counter = 0;
    
    printf("\nGenerating requests for disaster regions...\n");
    for (int i = 0; i < nreg; i++)
    {
        if (!is_disaster[i]) continue;

        // For each resource, create a request
        for (int r = 0; r < nres; r++)
        {
            Request *req = malloc(sizeof(Request));
            snprintf(req->id, IDLEN, "RQ_%d", req_counter++);
            strncpy(req->region_id, regions[i].id, IDLEN - 1);
            req->region_id[IDLEN - 1] = '\0';
            strncpy(req->resource_id, resources[r].id, IDLEN - 1);
            req->resource_id[IDLEN - 1] = '\0';
            
            // Request quantity based on population (can be adjusted)
            req->qty_needed = regions[i].population;

            int priority = regions[i].severity * 1000 + regions[i].population;
            heap_insert(h, req, priority);
        }
    }
    printf("✓ Generated %d requests\n", req_counter);

    //Run Allocation Process
    printf("\n=== Starting Allocation Process ===\n");
    run_allocator(h, g, hm, regions, nreg, resources, nres);

    // Cleanup Requests in Heap
    Request *req;
    while ((req = heap_pop(h)) != NULL)
        free(req);
    heap_free(h);

    // Cleanup Other Resources
    hm_free(hm);
    graph_free(g);
    free(is_disaster);
    free(resources);
    free(regions);

    printf("\n=== Allocation Complete ===\n");
    printf("✓ Results written to ../data/report.txt\n\n");
    return 0;
}