#include "../include/graph.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
Graph *graph_create(int n)
{
    Graph *g = malloc(sizeof(Graph));
    g->n = n;
    g->regions = NULL;
    g->adj = calloc(n, sizeof(Edge *));
    return g;
}
static int find_region_index(Graph *g, const char *id)
{
    if (!g->regions)
        return -1;
    for (int i = 0; i < g->n; i++)
        if (strcmp(g->regions[i].id, id) == 0)
            return i;
    return -1;
}
void graph_add_edge(Graph *g, const char *from, const char *to, int cost)
{
    int fi = find_region_index(g, from);
    int ti = find_region_index(g, to);
    if (fi < 0 || ti < 0)
        return;
    Edge *e = malloc(sizeof(Edge));
    e->to = ti;
    e->cost = cost;
    e->next = g->adj[fi];
    g->adj[fi] = e;
}
int graph_shortest_path(Graph *g, const char *src, const char *dst, char ***path_out, int *path_len, int *dist_out)
{
    int n = g->n;
    int si = find_region_index(g, src), di = find_region_index(g, dst);
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
    for (;;)
    {
        int u = -1;
        int best = INT_MAX;
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
        return 2;
    }
    int cur = di;
    int plen = 0;
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
void graph_free(Graph *g)
{
    for (int i = 0; i < g->n; i++)
    {
        Edge *e = g->adj[i];
        while (e)
        {
            Edge *n = e->next;
            free(e);
            e = n;
        }
    }
    free(g->adj);
    free(g);
}