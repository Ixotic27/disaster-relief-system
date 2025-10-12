#include "../include/graph.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include "graph.h"

// Create a new graph with n regions
Graph *graph_create(int n)
{
    Graph *g = malloc(sizeof(Graph));
    g->n = n;
    g->regions = NULL;                  // Will link to loaded regions later
    g->adj = calloc(n, sizeof(Edge *)); // adjacency list
    return g;
}

// Find the index of a region by NAME
static int find_region_index_by_name(Graph *g, const char *name)
{
    if (!g->regions)
        return -1;
    for (int i = 0; i < g->n; i++)
        if (strcmp(g->regions[i].name, name) == 0)
            return i;
    return -1;
}

// Find the index of a region by ID
static int find_region_index_by_id(Graph *g, const char *id)
{
    if (!g->regions)
        return -1;
    for (int i = 0; i < g->n; i++)
        if (strcmp(g->regions[i].id, id) == 0)
            return i;
    return -1;
}

// Add a directed edge from "from" -> "to" with cost
// Uses name matching (for edges.txt which has region names)
void graph_add_edge(Graph *g, const char *from, const char *to, int cost)
{
    int fi = find_region_index_by_name(g, from);
    int ti = find_region_index_by_name(g, to);
    if (fi < 0 || ti < 0)
    {
        printf("Warning: Could not find edge %s -> %s\n", from, to);
        return;
    }

    Edge *e = malloc(sizeof(Edge));
    e->to = ti;
    e->cost = cost;
    e->next = g->adj[fi];
    g->adj[fi] = e;
}

// Compute shortest path using Dijkstra
int graph_shortest_path(Graph *g, const char *src, const char *dst, char ***path_out, int *path_len, int *dist_out)
{
    int n = g->n;
    int si = find_region_index_by_id(g, src);      // src/dst are IDs (RG1, RG2, etc)
    int di = find_region_index_by_id(g, dst);
    if (si < 0 || di < 0)
        return 1;

    int *dist = malloc(sizeof(int) * n);
    int *prev = malloc(sizeof(int) * n);
    char *used = calloc(n, 1);

    for (int i = 0; i < n; i++)
    {
        dist[i] = INT_MAX;
        prev[i] = -1;
    }
    dist[si] = 0;

    // Dijkstra algorithm
    for (;;)
    {
        int u = -1, best = INT_MAX;
        for (int i = 0; i < n; i++)
            if (!used[i] && dist[i] < best)
            {
                best = dist[i];
                u = i;
            }
        if (u == -1)
            break;

        used[u] = 1;

        for (Edge *e = g->adj[u]; e; e = e->next)
        {
            int v = e->to;
            int nd = dist[u] + e->cost;
            if (nd < dist[v])
            {
                dist[v] = nd;
                prev[v] = u;
            }
        }
    }

    if (dist[di] == INT_MAX)
    {
        free(dist);
        free(prev);
        free(used);
        return 2; // No path
    }

    // Reconstruct path
    int plen = 0, cur = di;
    while (cur != -1)
    {
        plen++;
        cur = prev[cur];
    }

    char **path = malloc(sizeof(char *) * plen);
    cur = di;
    for (int i = plen - 1; i >= 0; i--)
    {
        path[i] = strdup(g->regions[cur].id);
        cur = prev[cur];
    }

    *path_out = path;
    *path_len = plen;
    *dist_out = dist[di];

    free(dist);
    free(prev);
    free(used);
    return 0;
}

// Free graph memory
void graph_free(Graph *g)
{
    if (!g)
        return;
    for (int i = 0; i < g->n; i++)
    {
        Edge *e = g->adj[i];
        while (e)
        {
            Edge *tmp = e->next;
            free(e);
            e = tmp;
        }
    }
    free(g->adj);
    free(g);
}