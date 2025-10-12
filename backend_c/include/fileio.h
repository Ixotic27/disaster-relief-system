#ifndef FILEIO_H
#define FILEIO_H

#include "types.h"
#include "graph.h"

int load_resources(const char *file_path, Resource **resources, int *nres);
int load_regions(const char *file_path, Region **regions, int *nreg);
int load_edges(const char *file_path, Graph *g);

#endif // FILEIO_H
