#include "../include/fileio.h"
#include "../include/types.h"
#include "../include/graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *strip(char *s)
{
    while (*s == ' ' || *s == '\t')
        s++;
    char *e = s + strlen(s) - 1;
    while (e >= s && (*e == '\n' || *e == '\r' || *e == ' ' || *e == '\t'))
        *e-- = 0;
    return s;
}

int load_resources(const char *path, Resource **arr, int *n)
{
    FILE *f = fopen(path, "r");
    if (!f)
        return 1;
    char buf[256];
    int cap = 8;
    *n = 0;
    *arr = malloc(sizeof(Resource) * cap);
    fgets(buf, sizeof(buf), f);
    while (fgets(buf, sizeof(buf), f))
    {
        char *p = strdup(buf);
        if (!p) continue;
        char *id = strtok(p, ",");
        char *name = strtok(NULL, ",");
        char *qty = strtok(NULL, ",");
        if (!id || !name || !qty)
        {
            free(p);
            continue;
        }
        if (*n >= cap)
        {
            cap *= 2;
            *arr = realloc(*arr, sizeof(Resource) * cap);
        }
        strncpy((*arr)[*n].id, strip(id), IDLEN - 1);
        (*arr)[*n].id[IDLEN - 1] = 0;
        strncpy((*arr)[*n].name, strip(name), 63);
        (*arr)[*n].name[63] = 0;
        (*arr)[*n].quantity = atoi(strip(qty));
        (*n)++;
        free(p);
    }
    fclose(f);
    return 0;
}

int load_regions(const char *path, Region **arr, int *n)
{
    FILE *f = fopen(path, "r");
    if (!f)
        return 1;
    char buf[256];
    int cap = 8;
    *n = 0;
    *arr = malloc(sizeof(Region) * cap);
    fgets(buf, sizeof(buf), f);
    while (fgets(buf, sizeof(buf), f))
    {
        char *p = strdup(buf);
        if (!p) continue;
        char *id = strtok(p, ",");
        char *name = strtok(NULL, ",");
        char *sev = strtok(NULL, ",");
        if (!id || !name || !sev)
        {
            free(p);
            continue;
        }
        if (*n >= cap)
        {
            cap *= 2;
            *arr = realloc(*arr, sizeof(Region) * cap);
        }
        strncpy((*arr)[*n].id, strip(id), IDLEN - 1);
        (*arr)[*n].id[IDLEN - 1] = 0;
        strncpy((*arr)[*n].name, strip(name), 63);
        (*arr)[*n].name[63] = 0;
        (*arr)[*n].severity = atoi(strip(sev));
        (*n)++;
        free(p);
    }
    fclose(f);
    return 0;
}

int load_requests(const char *path, Request **arr, int *n)
{
    FILE *f = fopen(path, "r");
    if (!f)
        return 1;
    char buf[256];
    int cap = 8;
    *n = 0;
    *arr = malloc(sizeof(Request) * cap);
    fgets(buf, sizeof(buf), f);
    while (fgets(buf, sizeof(buf), f))
    {
        char *p = strdup(buf);
        if (!p) continue;
        char *rid = strtok(p, ",");
        char *ridreg = strtok(NULL, ",");
        char *resid = strtok(NULL, ",");
        char *qty = strtok(NULL, ",");
        if (!rid || !ridreg || !resid || !qty)
        {
            free(p);
            continue;
        }
        if (*n >= cap)
        {
            cap *= 2;
            *arr = realloc(*arr, sizeof(Request) * cap);
        }
        strncpy((*arr)[*n].id, strip(rid), IDLEN - 1);
        (*arr)[*n].id[IDLEN - 1] = 0;
        strncpy((*arr)[*n].region_id, strip(ridreg), IDLEN - 1);
        (*arr)[*n].region_id[IDLEN - 1] = 0;
        strncpy((*arr)[*n].resource_id, strip(resid), IDLEN - 1);
        (*arr)[*n].resource_id[IDLEN - 1] = 0;
        (*arr)[*n].qty_needed = atoi(strip(qty));
        (*n)++;
        free(p);
    }
    fclose(f);
    return 0;
}

int load_edges(const char *path, void *graph)
{
    Graph *g = (Graph *)graph;
    FILE *f = fopen(path, "r");
    if (!f)
        return 1;
    char buf[256];
    fgets(buf, sizeof(buf), f);
    while (fgets(buf, sizeof(buf), f))
    {
        char *p = strdup(buf);
        if (!p) continue;
        char *from = strtok(p, ",");
        char *to = strtok(NULL, ",");
        char *cost = strtok(NULL, ",");
        if (!from || !to || !cost)
        {
            free(p);
            continue;
        }
        graph_add_edge(g, strip(from), strip(to), atoi(strip(cost)));
        free(p);
    }
    fclose(f);
    return 0;
}