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

// main function now accepts command-line arguments for the input file
int main(int argc, char **argv)
{
    // --- MODIFICATION 1: Check for input file argument ---
    if (argc < 2) {
        fprintf(stderr, "Error: Missing input file path.\n");
        fprintf(stderr, "Usage: %s <path_to_input_file>\n", argv[0]);
        return 1;
    }
    const char *inputFilePath = argv[1];

    // --- MODIFICATION 2: Open the input file provided by Python ---
    FILE *inputFile = fopen(inputFilePath, "r");
    if (!inputFile) {
        // Use perror for more descriptive file errors
        perror("CRITICAL ERROR: Failed to open input file");
        return 1;
    }

    // This part remains the same
    const char *dataPath = "../../data";
    char filePath[256];
    Resource *resources = NULL;
    Region *regions = NULL;
    int nres = 0, nreg = 0;

    // === Load Resources ===
    snprintf(filePath, sizeof(filePath), "%s/resources.txt", dataPath);
    if (load_resources(filePath, &resources, &nres) != 0) {
        fprintf(stderr, "Error: Failed to load resources from %s\n", filePath);
        fclose(inputFile); // Close the file before exiting
        return 1;
    }

    // === Load Regions ===
    snprintf(filePath, sizeof(filePath), "%s/regions.txt", dataPath);
    if (load_regions(filePath, &regions, &nreg) != 0) {
        fprintf(stderr, "Error: Failed to load regions from %s\n", filePath);
        fclose(inputFile);
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
        fprintf(stderr, "Warning: Failed to load edges from %s\n", filePath);

    // Default severity = 0 for all regions
    for (int i = 0; i < nreg; i++)
        regions[i].severity = 0;

    // --- MODIFICATION 3: Read from the input file using fscanf ---
    int disasterCount = 0;
    // Replace scanf("%d", &disasterCount)
    if (fscanf(inputFile, "%d", &disasterCount) != 1 || disasterCount < 0 || disasterCount > nreg) {
        fprintf(stderr, "Error: Invalid number of disaster regions in input file.\n");
        fclose(inputFile);
        return 1;
    }

    // === Read affected regions and their severities from the file ===
    for (int i = 0; i < disasterCount; i++) {
        int region_id = -1, severity = -1;
        // Replace scanf("%d %d", &region_id, &severity)
        if (fscanf(inputFile, "%d %d", &region_id, &severity) != 2) {
            fprintf(stderr, "Error: Malformed region/severity line in input file.\n");
            fclose(inputFile);
            return 1;
        }

        // Find the region index from its ID. Note: region IDs in your app start from 1.
        int region_idx = -1;
        for (int j = 0; j < nreg; j++) {
            // Assuming region IDs are strings, so we convert the read int to string to compare
            char id_str[10];
            snprintf(id_str, 10, "%d", region_id);
            if (strcmp(regions[j].id, id_str) == 0) {
                region_idx = j;
                break;
            }
        }
        
        if (region_idx == -1) {
            fprintf(stderr, "Error: Region with ID %d not found.\n", region_id);
            continue; // Skip this invalid region
        }

        if (severity < 1 || severity > 10) {
            fprintf(stderr, "Warning: Invalid severity %d for region %d. Defaulting to 1.\n", severity, region_id);
            severity = 1;
        }
        regions[region_idx].severity = severity;
    }

    // --- MODIFICATION 4: Close the input file ---
    fclose(inputFile);

    // === Prepare Heap of Disaster Requests ===
    Heap *h = heap_create(disasterCount * nres + 5);
    int req_counter = 0;
    for (int i = 0; i < nreg; i++) {
        if (regions[i].severity > 0) { // Check if it's a disaster region
            for (int r = 0; r < nres; r++) {
                Request *req = malloc(sizeof(Request));
                snprintf(req->id, IDLEN, "RQ_%d", req_counter++);
                strncpy(req->region_id, regions[i].id, IDLEN - 1);
                req->region_id[IDLEN - 1] = '\0';
                strncpy(req->resource_id, resources[r].id, IDLEN - 1);
                req->resource_id[IDLEN - 1] = '\0';
                
                // Simplified logic: request quantity based on severity
                req->qty_needed = regions[i].severity * 100;

                int priority = regions[i].severity * 1000 + regions[i].population;
                heap_insert(h, req, priority);
            }
        }
    }
    
    // === Allocation Process and Report Generation ===
    // This part assumes run_allocator does the main work and generates the report
    run_allocator(h, g, hm, regions, nreg, resources, nres);


    // === Cleanup ===
    Request *r;
    while ((r = heap_pop(h)) != NULL)
        free(r);
    heap_free(h);
    hm_free(hm);
    graph_free(g);
    free(resources);
    free(regions);

    return 0; // SUCCESS!
}
