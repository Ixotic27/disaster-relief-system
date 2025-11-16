#include "../include/allocation.h"
#include "../include/report.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Per-region resource stock tracking
typedef struct {
    int region_idx;
    int stock[10];  // Max 10 resource types
} RegionStock;

static int find_region_index_internal(Region *regions, int n, const char *rid) {
    for (int i = 0; i < n; i++) {
        if (strcmp(regions[i].id, rid) == 0)
            return i;
    }
    return -1;
}

static int find_resource_index(Resource *resources, int n, const char *rid) {
    for (int i = 0; i < n; i++) {
        if (strcmp(resources[i].id, rid) == 0)
            return i;
    }
    return -1;
}

int region_index(Region *regions, int n, const char *rid) {
    return find_region_index_internal(regions, n, rid);
}

int region_severity(Region *regions, int n, const char *rid) {
    int idx = find_region_index_internal(regions, n, rid);
    return (idx >= 0) ? regions[idx].severity : 0;
}

// Load per-region resource stocks from region_resources.txt
RegionStock* load_region_stocks(const char *path, Region *regions, int nreg, int nres, int *stock_count) {
    FILE *f = fopen(path, "r");
    if (!f) {
        printf("Warning: Could not load region_resources.txt, using global pool\n");
        return NULL;
    }

    RegionStock *stocks = calloc(nreg, sizeof(RegionStock));
    for (int i = 0; i < nreg; i++) {
        stocks[i].region_idx = i;
        for (int r = 0; r < nres; r++) {
            stocks[i].stock[r] = 0;
        }
    }

    char buf[256];
    while (fgets(buf, sizeof(buf), f)) {
        if (buf[0] == '#' || buf[0] == '\n') continue;
        
        int region_idx;
        char region_id[32], resource_id[32], resource_name[64];
        int quantity;
        
        if (sscanf(buf, "%d,%[^,],%[^,],%[^,],%d", 
                   &region_idx, region_id, resource_id, resource_name, &quantity) == 5) {
            
            // Find resource index
            int res_idx = -1;
            if (strcmp(resource_id, "R1") == 0) res_idx = 0;
            else if (strcmp(resource_id, "R2") == 0) res_idx = 1;
            else if (strcmp(resource_id, "R3") == 0) res_idx = 2;
            else if (strcmp(resource_id, "R4") == 0) res_idx = 3;
            
            if (res_idx >= 0 && region_idx < nreg) {
                stocks[region_idx].stock[res_idx] = quantity;
            }
        }
    }
    
    fclose(f);
    *stock_count = nreg;
    
    printf("✓ Loaded per-region resource stocks\n");
    for (int i = 0; i < nreg; i++) {
        if (regions[i].severity == 0 && 
            (stocks[i].stock[0] > 0 || stocks[i].stock[1] > 0 || 
             stocks[i].stock[2] > 0 || stocks[i].stock[3] > 0)) {
            printf("  %s: R=%d, W=%d, B=%d, M=%d\n", 
                   regions[i].name, 
                   stocks[i].stock[0], stocks[i].stock[1], 
                   stocks[i].stock[2], stocks[i].stock[3]);
        }
    }
    
    return stocks;
}

void run_allocator(Heap *h, Graph *g, HashMap *hm, Region *regions, int nreg, Resource *resources, int nres) {
    Request *r;
    AllocationReport *report = allocation_report_create(nreg);

    printf("\nStarting Resource Allocation Process...\n");
    init_report("../data/report.txt");

    // Load per-region stocks
    int stock_count = 0;
    RegionStock *region_stocks = load_region_stocks("../data/region_resources.txt", regions, nreg, nres, &stock_count);
    
    if (!region_stocks) {
        printf("Error: Could not load region stocks\n");
        return;
    }

    // Initialize region results for affected regions
    for (int i = 0; i < nreg; i++) {
        if (regions[i].severity > 0) {
            RegionAllocationResult region_result;
            strncpy(region_result.region_id, regions[i].id, IDLEN - 1);
            region_result.region_id[IDLEN - 1] = '\0';
            strncpy(region_result.region_name, regions[i].name, NAMELEN - 1);
            region_result.region_name[NAMELEN - 1] = '\0';
            region_result.severity = regions[i].severity;
            region_result.population = regions[i].population;
            region_result.resource_count = nres;

            for (int r = 0; r < nres; r++) {
                strncpy(region_result.resources[r].resource_id, resources[r].id, IDLEN - 1);
                region_result.resources[r].resource_id[IDLEN - 1] = '\0';
                strncpy(region_result.resources[r].resource_name, resources[r].name, NAMELEN - 1);
                region_result.resources[r].resource_name[NAMELEN - 1] = '\0';
                region_result.resources[r].allocated = 0;
                region_result.resources[r].needed = regions[i].population;
                strcpy(region_result.resources[r].status, "NOT_ALLOCATED");
            }

            allocation_report_add_region(report, &region_result);
        }
    }

    // Process requests
    while ((r = heap_pop(h)) != NULL) {
        int res_idx = find_resource_index(resources, nres, r->resource_id);
        
        if (res_idx < 0) {
            log_allocation(r->id, r->region_id, r->resource_id, 0, "", 0, "RESOURCE_NOT_FOUND");
            free(r);
            continue;
        }

        int disaster_idx = find_region_index_internal(regions, nreg, r->region_id);
        if (disaster_idx < 0) {
            log_allocation(r->id, r->region_id, r->resource_id, 0, "", 0, "REGION_NOT_FOUND");
            free(r);
            continue;
        }

        Region *affected = &regions[disaster_idx];

        // Find all safe regions sorted by distance
        typedef struct {
            int idx;
            int distance;
            char **path;
            int plen;
        } SupplierOption;
        
        SupplierOption suppliers[100];
        int supplier_count = 0;

        for (int i = 0; i < nreg; i++) {
            if (regions[i].severity == 0 && region_stocks[i].stock[res_idx] > 0) {
                char **temp_path = NULL;
                int temp_plen = 0;
                int temp_dist = 0;
                
                if (graph_shortest_path(g, regions[i].id, r->region_id, &temp_path, &temp_plen, &temp_dist) == 0) {
                    suppliers[supplier_count].idx = i;
                    suppliers[supplier_count].distance = temp_dist;
                    suppliers[supplier_count].path = temp_path;
                    suppliers[supplier_count].plen = temp_plen;
                    supplier_count++;
                }
            }
        }

        if (supplier_count == 0) {
            log_allocation(r->id, r->region_id, r->resource_id, 0, "NO_PATH", 0, "NO_SUPPLIER");
            free(r);
            continue;
        }

        // Sort suppliers by distance (bubble sort for simplicity)
        for (int i = 0; i < supplier_count - 1; i++) {
            for (int j = 0; j < supplier_count - i - 1; j++) {
                if (suppliers[j].distance > suppliers[j + 1].distance) {
                    SupplierOption temp = suppliers[j];
                    suppliers[j] = suppliers[j + 1];
                    suppliers[j + 1] = temp;
                }
            }
        }

        // Allocate from nearest supplier
        int supplier_idx = suppliers[0].idx;
        int available = region_stocks[supplier_idx].stock[res_idx];
        int allocated = (available >= r->qty_needed) ? r->qty_needed : available;
        
        region_stocks[supplier_idx].stock[res_idx] -= allocated;

        // Build route string
        char route[512];
        route[0] = '\0';
        
        for (int i = 0; i < suppliers[0].plen; i++) {
            int rid_idx = find_region_index_internal(regions, nreg, suppliers[0].path[i]);
            if (rid_idx >= 0) {
                strcat(route, regions[rid_idx].name);
            }
            if (i < suppliers[0].plen - 1) strcat(route, " -> ");
        }

        const char *status;
        if (allocated == 0) status = "FAILED";
        else if (allocated < r->qty_needed) status = "PARTIAL";
        else status = "FULFILLED";

        log_allocation(r->id, r->region_id, r->resource_id, allocated, route, suppliers[0].distance, status);

        // Update report
        for (int i = 0; i < report->region_count; i++) {
            if (strcmp(report->regions[i].region_id, r->region_id) == 0) {
                for (int j = 0; j < report->regions[i].resource_count; j++) {
                    if (strcmp(report->regions[i].resources[j].resource_id, r->resource_id) == 0) {
                        report->regions[i].resources[j].allocated += allocated;
                        strcpy(report->regions[i].resources[j].status, status);
                        break;
                    }
                }
                break;
            }
        }

        // Add route to report
        if (allocated > 0) {
            ResourceRoute route_info;
            strncpy(route_info.source_region_id, regions[supplier_idx].id, IDLEN - 1);
            route_info.source_region_id[IDLEN - 1] = '\0';
            strncpy(route_info.source_region_name, regions[supplier_idx].name, NAMELEN - 1);
            route_info.source_region_name[NAMELEN - 1] = '\0';
            strncpy(route_info.dest_region_id, r->region_id, IDLEN - 1);
            route_info.dest_region_id[IDLEN - 1] = '\0';
            strncpy(route_info.dest_region_name, affected->name, NAMELEN - 1);
            route_info.dest_region_name[NAMELEN - 1] = '\0';
            strncpy(route_info.resource_id, r->resource_id, IDLEN - 1);
            route_info.resource_id[IDLEN - 1] = '\0';
            strncpy(route_info.resource_name, resources[res_idx].name, NAMELEN - 1);
            route_info.resource_name[NAMELEN - 1] = '\0';
            route_info.quantity = allocated;
            route_info.distance = suppliers[0].distance;
            strncpy(route_info.route, route, 511);
            route_info.route[511] = '\0';

            allocation_report_add_route(report, &route_info);
        }

        // Free paths
        for (int i = 0; i < supplier_count; i++) {
            for (int j = 0; j < suppliers[i].plen; j++) {
                free(suppliers[i].path[j]);
            }
            free(suppliers[i].path);
        }

        free(r);
    }

    printf("\nAll allocations completed.\n");
    print_allocation_summary_report(report, resources, nres);
    
    free(region_stocks);
    allocation_report_free(report);
}