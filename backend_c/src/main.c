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

    // === Load Resources ===
    snprintf(filePath, sizeof(filePath), "%s/resources.txt", dataPath);
    if (load_resources(filePath, &resources, &nres) != 0)
    {
        fprintf(stderr, "Error: Failed to load resources\n");
        return 1;
    }
    printf("Loaded %d resources\n", nres);

    // === Load Regions ===
    snprintf(filePath, sizeof(filePath), "%s/regions.txt", dataPath);
    if (load_regions(filePath, &regions, &nreg) != 0)
    {
        fprintf(stderr, "Error: Failed to load regions\n");
        return 1;
    }
    printf("Loaded %d regions\n", nreg);

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

    // === Print all regions ===
    printf("\n=== Disaster Resource Allocation System ===\n\n");
    printf("List of Regions:\n");
    for (int i = 0; i < nreg; i++)
        printf(" [%d] %s (id: %s)\n", i + 1, regions[i].name, regions[i].id);

    // Default severity = 0 for all regions
    for (int i = 0; i < nreg; i++)
        regions[i].severity = 0;

    // Ask number of affected regions FIRST
    int disasterCount = 0;
    printf("\nEnter number of disaster-affected regions: ");
    if (scanf("%d", &disasterCount) != 1 || disasterCount < 0 || disasterCount > nreg)
    {
        printf("Invalid number. Exiting.\n");
        return 1;
    }
    while (getchar() != '\n'); // flush buffer

    int *is_disaster = calloc(nreg, sizeof(int));
    if (!is_disaster)
    {
        return 1;
    }

    // === Select affected regions and ONLY ask severity for affected ones ===
    for (int i = 0; i < disasterCount; i++)
    {
        int count = -1;
        printf("\nSelect disaster region #%d (enter region number): ", i + 1);
        if (scanf("%d", &count) != 1 || count < 1 || count > nreg)
        {
            printf("Invalid selection. Try again.\n");
            while (getchar() != '\n');
            i--;
            continue;
        }
        while (getchar() != '\n'); // flush buffer

        int idx = count - 1;
        if (is_disaster[idx])
        {
            printf("Already selected. Pick another.\n");
            i--;
            continue;
        }

        is_disaster[idx] = 1;

        // ONLY ask severity for affected regions
        int sev = 0;
        printf("Enter severity for %s (1-10): ", regions[idx].name);
        if (scanf("%d", &sev) != 1 || sev < 1 || sev > 10)
        {
            printf("Invalid input. Using default severity = 1.\n");
            sev = 1;
            while (getchar() != '\n');
        }
        while (getchar() != '\n'); // flush buffer
        regions[idx].severity = sev;
    }

    // === Print safe regions ===
    printf("\nSafe regions:\n");
    for (int i = 0; i < nreg; i++)
        if (!is_disaster[i])
            printf("  - %s (id: %s)\n", regions[i].name, regions[i].id);

    // === Resource allocation for safe regions ===
    printf("\nDo you want to manually enter resources for SAFE regions? (y/n): ");
    char yn;
    scanf("%c", &yn);
    while (getchar() != '\n'); // flush buffer

    int **stock = malloc(sizeof(int *) * nres);
    for (int r = 0; r < nres; r++)
        stock[r] = calloc(nreg, sizeof(int));

    if (yn == 'y' || yn == 'Y')
    {
        for (int i = 0; i < nreg; i++)
        {
            if (is_disaster[i]) continue;
            printf("\n-- Safe region: %s --\n", regions[i].name);
            for (int r = 0; r < nres; r++)
            {
                printf("  Available %s (%s): ", resources[r].name, resources[r].id);
                int q = 0;
                if (scanf("%d", &q) != 1 || q < 0)
                {
                    printf("Invalid, assuming 0.\n");
                    q = 0;
                    while (getchar() != '\n');
                }
                stock[r][i] = q;
            }
        }
    }
    else
    {
        int safeCount = 0;
        for (int i = 0; i < nreg; i++)
            if (!is_disaster[i]) safeCount++;
        if (safeCount == 0) safeCount = 1;

        for (int r = 0; r < nres; r++)
        {
            int per = resources[r].quantity / safeCount;
            int rem = resources[r].quantity % safeCount;
            for (int i = 0; i < nreg; i++)
            {
                if (is_disaster[i]) { stock[r][i] = 0; continue; }
                stock[r][i] = per + (rem > 0 ? 1 : 0);
                if (rem > 0) rem--;
            }
        }
        printf("\nDistributed global resources to safe regions.\n");
    }

    // === Save disaster_config.txt ===
    snprintf(filePath, sizeof(filePath), "%s/disaster_config.txt", dataPath);
    FILE *df = fopen(filePath, "w");
    if (df)
    {
        fprintf(df, "#region_index,region_id,region_name,severity,population\n");
        for (int i = 0; i < nreg; i++)
            if (is_disaster[i])
                fprintf(df, "%d,%s,%s,%d,%d\n", i, regions[i].id, regions[i].name, regions[i].severity, regions[i].population);
        fclose(df);
        printf("Saved disaster_config.txt\n");
    }

    // === Save region_resources.txt ===
    snprintf(filePath, sizeof(filePath), "%s/region_resources.txt", dataPath);
    FILE *rf = fopen(filePath, "w");
    if (rf)
    {
        fprintf(rf, "#region_index,region_id,resource_id,resource_name,quantity\n");
        for (int i = 0; i < nreg; i++)
            for (int r = 0; r < nres; r++)
                if (stock[r][i] > 0)
                    fprintf(rf, "%d,%s,%s,%s,%d\n", i, regions[i].id, resources[r].id, resources[r].name, stock[r][i]);
        fclose(rf);
        printf("Saved region_resources.txt\n");
    }

    // === Prepare Heap of Disaster Requests ===
    // Create a request for EACH resource in each affected region
    Heap *h = heap_create(disasterCount * nres + 5);
    int req_counter = 0;
    
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
            strncpy(req->resource_id, resources[r].id, IDLEN - 1);  // ASSIGN RESOURCE ID
            req->resource_id[IDLEN - 1] = '\0';
            
            printf("DEBUG: Request %s - Region: %s, Resource: %s\n", req->id, req->region_id, req->resource_id);
            
            req->qty_needed = regions[i].population;

            int priority = regions[i].severity * 1000 + regions[i].population;
            heap_insert(h, req, priority);
        }
    }

    // === Generate Report ===
    snprintf(filePath, sizeof(filePath), "%s/report.txt", dataPath);

    // === Allocation Process ===
    run_allocator(h, g, hm, regions, nreg, resources, nres);

    // === Cleanup Requests in Heap ===
    Request *r;
    while ((r = heap_pop(h)) != NULL)
        free(r);
    heap_free(h);

    // === Cleanup Other Resources ===
    hm_free(hm);
    graph_free(g);
    for (int r = 0; r < nres; r++) free(stock[r]);
    free(stock);
    free(is_disaster);
    free(resources);
    free(regions);

    printf("\nSetup complete. Allocation finished (see report.txt).\n");
    return 0;
}