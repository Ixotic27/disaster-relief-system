#ifndef GRAPH_H
#define GRAPH_H
#include "types.h"
typedef struct Edge
{
    int to;
    int cost;
    struct Edge *next;
} Edge;
typedef struct
{
    Region *regions;
    int n;
    Edge **adj;
} Graph;
Graph *graph_create(int n);
void graph_add_edge(Graph *g, const char *from, const char *to, int cost);
int graph_shortest_path(Graph *g, const char *src, const char *dst, char ***path_out, int *path_len, int *dist_out);
void graph_free(Graph *g);
#endif