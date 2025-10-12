#ifndef GRAPH_H
#define GRAPH_H

#include "types.h"

typedef struct Edge {
    int to;
    int cost;
    struct Edge *next;
} Edge;

typedef struct {
    int n;        // number of nodes
    Edge **adj;   // adjacency list
    Region *regions; // pointer to regions array
} Graph;

// Graph functions
Graph *graph_create(int n);
void graph_free(Graph *g);
void graph_add_edge(Graph *g, const char *from, const char *to, int cost);
int graph_shortest_path(Graph *g, const char *src, const char *dst, char ***path_out, int *path_len, int *dist_out);

#endif // GRAPH_H
