#include "../include/allocation.h"
#include "../include/report.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================
// Helper: Find region index by ID
// ============================================================
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

// ============================================================
// Main Resource Allocator with Report Collection
// ============================================================

void run_allocator(Heap *h, Graph *g, HashMap *hm, Region *regions, int nreg, Resource *resources, int nres) {
    Request *r;
    AllocationReport *report = allocation_report_create(nreg);

    printf("\nStarting Resource Allocation Process...\n");

    // Initialize report file at the beginning (write headers only)
    init_report("../data/report.txt");

    // Count affected regions first
    int affected_count = 0;
    for (int i = 0; i < nreg; i++) {
        if (regions[i].severity > 0) {
            affected_count++;
        }
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
            region_result.resource_count = 0;

            // Initialize all resources as not allocated
            for (int r = 0; r < nres; r++) {
                strncpy(region_result.resources[r].resource_id, resources[r].id, IDLEN - 1);
                region_result.resources[r].resource_id[IDLEN - 1] = '\0';
                strncpy(region_result.resources[r].resource_name, resources[r].name, NAMELEN - 1);
                region_result.resources[r].resource_name[NAMELEN - 1] = '\0';
                region_result.resources[r].allocated = 0;
                region_result.resources[r].needed = regions[i].population;
                strcpy(region_result.resources[r].status, "NOT_ALLOCATED");
            }
            region_result.resource_count = nres;

            allocation_report_add_region(report, &region_result);
        }
    }

    // Process requests
    while ((r = heap_pop(h)) != NULL) {
        Resource *res = hm_get(hm, r->resource_id);

        // Case 1: Resource not found
        if (!res) {
            log_allocation(r->id, r->region_id, r->resource_id, 0, "", 0, "RESOURCE_NOT_FOUND");
            free(r);
            continue;
        }

        // Case 2: Resource is out of stock
        if (res->quantity <= 0) {
            log_allocation(r->id, r->region_id, r->resource_id, 0, "", 0, "OUT_OF_STOCK");
            free(r);
            continue;
        }

        // ======================================================
        // Step 1: Find affected region details
        // ======================================================
        int idx = find_region_index_internal(regions, nreg, r->region_id);
        if (idx < 0) {
            log_allocation(r->id, r->region_id, r->resource_id, 0, "", 0, "REGION_NOT_FOUND");
            free(r);
            continue;
        }

        Region *affected = &regions[idx];

        // ======================================================
        // Step 2: Proportional Allocation by Population
        // ======================================================
        int allocated = 0;
        if (affected->population > 0) {
            if (res->quantity >= r->qty_needed) {
                allocated = r->qty_needed;
                res->quantity -= allocated;
            } else {
                allocated = res->quantity;
                res->quantity = 0;
            }
        } else {
            if (res->quantity >= r->qty_needed) {
                allocated = r->qty_needed;
                res->quantity -= allocated;
            } else {
                allocated = res->quantity;
                res->quantity = 0;
            }
        }

        // ======================================================
        // Step 3: Find shortest path from a safe region to disaster region
        // ======================================================
        char **path = NULL;
        int plen = 0;
        int dist = 0;
        int rc = -1;
        int supplier_idx = -1;
        
        // Try to find path from any safe region to the disaster region
        for (int i = 0; i < nreg; i++) {
            if (regions[i].severity == 0) {  // Safe region (potential supplier)
                rc = graph_shortest_path(g, regions[i].id, r->region_id, &path, &plen, &dist);
                if (rc == 0) {
                    supplier_idx = i;
                    break;  // Found a path from this supplier
                }
            }
        }

        char route[512];
        route[0] = '\0';

        if (rc == 0 && path != NULL) {
            for (int i = 0; i < plen; i++) {
                // Convert region ID to region name
                int rid_idx = find_region_index_internal(regions, nreg, path[i]);
                if (rid_idx >= 0) {
                    strcat(route, regions[rid_idx].name);
                } else {
                    strcat(route, path[i]);
                }
                if (i < plen - 1) strcat(route, " -> ");
                free(path[i]);
            }
            free(path);
        } else {
            strcpy(route, "NO_PATH");
        }

        // ======================================================
        // Step 4: Determine Status
        // ======================================================
        const char *status;
        if (allocated == 0)
            status = "FAILED";
        else if (allocated < r->qty_needed)
            status = (strcmp(route, "NO_PATH") == 0) ? "PARTIAL_NO_PATH" : "PARTIAL";
        else
            status = "FULFILLED";

        // ======================================================
        // Step 5: Log Allocation (CSV format) - APPENDS to file
        // ======================================================
        log_allocation(
            r->id,
            r->region_id,
            r->resource_id,
            allocated,
            route,
            dist,
            status
        );

        // ======================================================
        // Step 6: Update Report with Allocation Result
        // ======================================================
        int res_idx = find_resource_index(resources, nres, r->resource_id);
        
        for (int i = 0; i < report->region_count; i++) {
            if (strcmp(report->regions[i].region_id, r->region_id) == 0) {
                // Find the resource in this region and update it
                for (int j = 0; j < report->regions[i].resource_count; j++) {
                    if (strcmp(report->regions[i].resources[j].resource_id, r->resource_id) == 0) {
                        report->regions[i].resources[j].allocated = allocated;
                        strcpy(report->regions[i].resources[j].status, status);
                        break;
                    }
                }
                break;
            }
        }

        // ======================================================
        // Step 7: Add Route to Report (if allocation was successful and path exists)
        // ======================================================
        if (allocated > 0 && strcmp(route, "NO_PATH") != 0 && supplier_idx >= 0) {
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
            strncpy(route_info.resource_name, res->name, NAMELEN - 1);
            route_info.resource_name[NAMELEN - 1] = '\0';
            route_info.quantity = allocated;
            route_info.distance = dist;
            strncpy(route_info.route, route, 511);
            route_info.route[511] = '\0';

            allocation_report_add_route(report, &route_info);
        }

        free(r);
    }

    printf("\nAll allocations completed.\n");

    // Print summary tables to terminal
    print_allocation_summary_report(report, resources, nres);

    allocation_report_free(report);
}