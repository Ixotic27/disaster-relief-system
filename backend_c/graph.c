#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAX 100

struct Graph {
    int vertices;
    int adj[MAX][MAX];
};

struct Graph* createGraph(int vertices) {
    struct Graph* g = (struct Graph*)malloc(sizeof(struct Graph));
    g->vertices = vertices;
    for (int i = 0; i < vertices; i++)
        for (int j = 0; j < vertices; j++)
            g->adj[i][j] = 0;
    return g;
}

void addEdge(struct Graph* g, int src, int dest, int weight) {
    g->adj[src][dest] = weight;
    g->adj[dest][src] = weight; // undirected graph
}

// Function to find the vertex with the minimum distance
int minDistance(int dist[], int visited[], int V) {
    int min = INT_MAX, min_index = -1;
    for (int v = 0; v < V; v++) {
        if (!visited[v] && dist[v] <= min) {
            min = dist[v];
            min_index = v;
        }
    }
    return min_index;
}

// Dijkst
