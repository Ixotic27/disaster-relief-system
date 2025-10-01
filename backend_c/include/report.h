#ifndef REPORT_H
#define REPORT_H
int init_report(const char *path);
int log_allocation(const char *req_id, const char *region_id, const char *resource_id, int allocated, const char *route, int cost, const char *status);
#endif
