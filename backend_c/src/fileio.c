#include "../include/fileio.h"
#include "../include/types.h"
#include "../include/graph.h"
#include "graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================
// Utility Functions
// ============================================================

// Removes spaces, tabs, and newlines from both ends
static char *trim(char *s) {
    while (*s == ' ' || *s == '\t') s++;
    char *end = s + strlen(s) - 1;
    while (end >= s && (*end == '\n' || *end == '\r' || *end == ' ' || *end == '\t'))
        *end-- = '\0';
    return s;
}

// Checks if a line is a comment or blank
static int skip_line(const char *s) {
    return (s[0] == '#' || s[0] == '\n' || s[0] == '\r' || s[0] == '\0');
}

// ============================================================
// Load Resources
// Format → ID,Name,Category,Quantity
// ============================================================
int load_resources(const char *path, Resource **arr, int *n) {
    FILE *f = fopen(path, "r");
    if (!f) {
        printf("Could not open %s\n", path);
        return 1;
    }

    char buf[256];
    int cap = 8;
    *n = 0;
    *arr = malloc(sizeof(Resource) * cap);

    printf("\nLoading resources from %s...\n", path);

    while (fgets(buf, sizeof(buf), f)) {
        if (skip_line(buf)) continue;

        char id[IDLEN], name[64], category[32];
        int qty;

        if (sscanf(buf, "%[^,],%[^,],%[^,],%d", id, name, category, &qty) == 4) {
            if (*n >= cap) {
                cap *= 2;
                *arr = realloc(*arr, sizeof(Resource) * cap);
            }

            strncpy((*arr)[*n].id, trim(id), IDLEN - 1);
            (*arr)[*n].id[IDLEN - 1] = '\0';
            strncpy((*arr)[*n].name, trim(name), 63);
            (*arr)[*n].name[63] = '\0';
            strncpy((*arr)[*n].category, trim(category), 31);
            (*arr)[*n].category[31] = '\0';
            (*arr)[*n].quantity = qty;

            (*n)++;
        }
    }

    fclose(f);
    printf("%d resources loaded successfully.\n", *n);
    return 0;
}

// ============================================================
// Load Regions (Districts)
// Format → ID,Name,Severity,Population
// ============================================================
int load_regions(const char *path, Region **arr, int *n) {
    FILE *f = fopen(path, "r");
    if (!f) {
        printf("Could not open %s\n", path);
        return 1;
    }

    char buf[256];
    int cap = 8;
    *n = 0;
    *arr = malloc(sizeof(Region) * cap);

    printf("\nLoading districts (regions) from %s...\n", path);

    while (fgets(buf, sizeof(buf), f)) {
        if (skip_line(buf)) continue;

        char id[IDLEN], name[64];
        int severity = 0, population = 0;

        // Parse: ID,Name,Severity,Population
        if (sscanf(buf, "%[^,],%[^,],%d,%d", id, name, &severity, &population) == 4) {
            if (*n >= cap) {
                cap *= 2;
                *arr = realloc(*arr, sizeof(Region) * cap);
            }

            strncpy((*arr)[*n].id, trim(id), IDLEN - 1);
            (*arr)[*n].id[IDLEN - 1] = '\0';
            strncpy((*arr)[*n].name, trim(name), 63);
            (*arr)[*n].name[63] = '\0';
            (*arr)[*n].population = population;
            (*arr)[*n].severity = severity;  // Load severity from file (will be 0 for safe regions)

            (*n)++;
        }
    }

    fclose(f);
    printf("Loaded %d districts successfully.\n", *n);
    return 0;
}

// ============================================================
// Load Requests
// Format → RequestID,RegionID,ResourceID,Quantity
// ============================================================
int load_requests(const char *path, Request **arr, int *n) {
    FILE *f = fopen(path, "r");
    if (!f) {
        printf("Could not open %s\n", path);
        return 1;
    }

    char buf[256];
    int cap = 8;
    *n = 0;
    *arr = malloc(sizeof(Request) * cap);

    printf("\nLoading requests from %s...\n", path);

    while (fgets(buf, sizeof(buf), f)) {
        if (skip_line(buf)) continue;

        char rid[IDLEN], region[IDLEN], res[IDLEN];
        int qty;

        if (sscanf(buf, "%[^,],%[^,],%[^,],%d", rid, region, res, &qty) == 4) {
            if (*n >= cap) {
                cap *= 2;
                *arr = realloc(*arr, sizeof(Request) * cap);
            }

            strncpy((*arr)[*n].id, trim(rid), IDLEN - 1);
            strncpy((*arr)[*n].region_id, trim(region), IDLEN - 1);
            strncpy((*arr)[*n].resource_id, trim(res), IDLEN - 1);
            (*arr)[*n].qty_needed = qty;

            (*n)++;
        }
    }

    fclose(f);
    printf("%d requests loaded successfully.\n", *n);
    return 0;
}

// ============================================================
// Load Edges (Graph Connections)
// Format → Region1,Region2,Distance
// ============================================================
int load_edges(const char *path, Graph *g){
    FILE *f = fopen(path, "r");
    if (!f) {
        printf("Could not open %s\n", path);
        return 1;
    }

    char buf[256];
    printf("\nLoading connections between districts from %s...\n", path);

    while (fgets(buf, sizeof(buf), f)) {
        if (skip_line(buf)) continue;

        char from[64], to[64];
        int dist;

        if (sscanf(buf, "%[^,],%[^,],%d", from, to, &dist) == 3) {
            trim(from); trim(to);
            graph_add_edge(g, from, to, dist);
            graph_add_edge(g, to, from, dist); // bidirectional
        }
    }

    fclose(f);
    printf("District routes loaded successfully.\n");
    return 0;
}