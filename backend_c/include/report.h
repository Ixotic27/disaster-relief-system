#ifndef REPORT_H
#define REPORT_H

#include "allocation.h"

// Original report functions
int init_report(const char *path);
int log_allocation(const char *req_id, const char *region_id, const char *resource_id, int allocated, const char *route, int cost, const char *status);

// Utility function to get terminal width
int get_terminal_width(void);
void print_separator(void);

// New functions for summary tables
AllocationReport *allocation_report_create(int max_regions);
void allocation_report_add_region(AllocationReport *report, RegionAllocationResult *region);
void allocation_report_add_route(AllocationReport *report, ResourceRoute *route);
void print_allocation_summary_report(AllocationReport *report, Resource *resources, int nres);
void generate_allocation_summary_report(const char *path, AllocationReport *report, Resource *resources, int nres);
void allocation_report_free(AllocationReport *report);

#endif // REPORT_H