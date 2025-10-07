#include "../include/report.h"
#include <stdio.h>
#include <time.h>
static char report_path[256];
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
int log_allocation(const char *req_id, const char *region_id, const char *resource_id, int allocated, const char *route, int cost, const char *status)
{
    FILE *f = fopen(report_path, "a");
    if (!f)
        return 1;
    time_t t = time(NULL);
    fprintf(f, "%s,%s,%s,%d,%s,%d,%s,%ld\n", req_id, region_id, resource_id, allocated, route, cost, status, (long)t);
    fclose(f);
    return 0;
}