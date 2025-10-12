#include "../include/report.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
#endif

static char report_path[256];

// Get terminal width
int get_terminal_width(void) {
    int width = 80;  // Default width
    
    #ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (GetConsoleScreenBufferInfo(hStdOut, &csbi)) {
            width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        }
    #else
        struct winsize w;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
            width = w.ws_col;
        }
    #endif
    
    return width;
}

// Print separator line based on terminal width
void print_separator(void) {
    int width = get_terminal_width();
    for (int i = 0; i < width - 1; i++) {
        printf("-");
    }
    printf("\n");
}

int init_report(const char *path)
{
    snprintf(report_path, sizeof(report_path), "%s", path);
    FILE *f = fopen(report_path, "w");
    if (!f)
        return 1;

    fprintf(f, "req_id,region_id,resource_id,allocated,route,cost,status,timestamp\n");
    fclose(f);
    return 0;
}

int log_allocation(const char *req_id, const char *region_id, const char *resource_id,
                   int allocated, const char *route, int cost, const char *status)
{
    FILE *f = fopen(report_path, "a");
    if (!f)
        return 1;

    time_t t = time(NULL);
    fprintf(f, "%s,%s,%s,%d,%s,%d,%s,%ld\n",
            req_id, region_id, resource_id, allocated, route, cost, status, (long)t);

    fclose(f);
    return 0;
}

// Create allocation report structure
AllocationReport *allocation_report_create(int max_regions)
{
    AllocationReport *report = malloc(sizeof(AllocationReport));
    report->regions = malloc(sizeof(RegionAllocationResult) * max_regions);
    report->routes = malloc(sizeof(ResourceRoute) * max_regions * MAX_RESOURCES * 2);
    report->region_count = 0;
    report->route_count = 0;
    return report;
}

void allocation_report_add_region(AllocationReport *report, RegionAllocationResult *region)
{
    if (report->region_count < 100) {
        memcpy(&report->regions[report->region_count], region, sizeof(RegionAllocationResult));
        report->region_count++;
    }
}

void allocation_report_add_route(AllocationReport *report, ResourceRoute *route)
{
    if (report->route_count < 500) {
        memcpy(&report->routes[report->route_count], route, sizeof(ResourceRoute));
        report->route_count++;
    }
}

void allocation_report_free(AllocationReport *report)
{
    if (!report) return;
    free(report->regions);
    free(report->routes);
    free(report);
}

// Print summary tables to terminal
void print_allocation_summary_report(AllocationReport *report, Resource *resources, int nres)
{
    int width = get_terminal_width();
    
    printf("\n\n");
    print_separator();
    printf("ALLOCATION SUMMARY REPORT\n");
    print_separator();
    printf("\n");

    // TABLE 1: Main Allocation Summary
    printf("TABLE 1: RESOURCE ALLOCATION BY REGION\n");
    print_separator();
    printf("%-20s ", "Region");
    for (int r = 0; r < nres; r++) {
        printf("| %-20s ", resources[r].name);
    }
    printf("| Severity\n");
    print_separator();

    for (int i = 0; i < report->region_count; i++) {
        RegionAllocationResult *reg = &report->regions[i];
        printf("%-20s ", reg->region_name);
        
        for (int r = 0; r < nres; r++) {
            int found = 0;
            for (int j = 0; j < reg->resource_count; j++) {
                if (strcmp(reg->resources[j].resource_id, resources[r].id) == 0) {
                    printf("| %d [%s]", reg->resources[j].allocated, reg->resources[j].status);
                    printf("%*s ", (int)(20 - strlen(reg->resources[j].status) - 
                                            (reg->resources[j].allocated > 99999 ? 6 : 
                                             reg->resources[j].allocated > 9999 ? 5 : 
                                             reg->resources[j].allocated > 999 ? 4 : 
                                             reg->resources[j].allocated > 99 ? 3 : 2)), "");
                    found = 1;
                    break;
                }
            }
            if (!found) {
                printf("| %-20s ", "0 [NOT_ALLOCATED]");
            }
        }
        printf("| %d\n", reg->severity);
    }
    print_separator();
    printf("\n");

    // TABLE 2: Per-Capita Allocation
    printf("TABLE 2: PER-CAPITA ALLOCATION\n");
    print_separator();
    printf("%-20s ", "Region");
    for (int r = 0; r < nres; r++) {
        printf("| %-18s ", resources[r].name);
    }
    printf("| Population\n");
    print_separator();

    for (int i = 0; i < report->region_count; i++) {
        RegionAllocationResult *reg = &report->regions[i];
        printf("%-20s ", reg->region_name);
        
        for (int r = 0; r < nres; r++) {
            double per_capita = 0.0;
            for (int j = 0; j < reg->resource_count; j++) {
                if (strcmp(reg->resources[j].resource_id, resources[r].id) == 0) {
                    if (reg->population > 0) {
                        per_capita = (double)reg->resources[j].allocated / reg->population;
                    }
                    break;
                }
            }
            printf("| %-18.2f ", per_capita);
        }
        printf("| %d\n", reg->population);
    }
    print_separator();
    printf("\n");

    // TABLE 3: Resource Distribution Routes
    printf("TABLE 3: RESOURCE DISTRIBUTION ROUTES\n");
    print_separator();
    printf("%-18s | %-18s | %-15s | %-10s | %-10s | %-40s\n",
            "Source", "Destination", "Resource", "Quantity", "Distance", "Route");
    print_separator();

    if (report->route_count == 0) {
        printf("%-18s | %-18s | %-15s | %-10s | %-10s | %-40s\n",
                "No routes", "available", "", "", "", "");
    } else {
        for (int i = 0; i < report->route_count; i++) {
            ResourceRoute *route = &report->routes[i];
            printf("%-18s | %-18s | %-15s | %-10d | %-10d | %-40s\n",
                    route->source_region_name,
                    route->dest_region_name,
                    route->resource_name,
                    route->quantity,
                    route->distance,
                    route->route);
        }
    }
    print_separator();
    printf("\n");
}

// Generate summary report as simple CSV (no formatting)
void generate_allocation_summary_report(const char *path, AllocationReport *report, Resource *resources, int nres)
{
    // This function is now empty - CSV data is already written via log_allocation()
    // This prevents writing formatted tables to the report file
    return;
}