#ifndef FILEIO_H
#define FILEIO_H
#include "types.h"
int load_resources(const char *path, Resource **arr, int *n);
int load_regions(const char *path, Region **arr, int *n);
int load_requests(const char *path, Request **arr, int *n);
int load_edges(const char *path, void *graph);
#endif
