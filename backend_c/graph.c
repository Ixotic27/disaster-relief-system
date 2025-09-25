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

// Find vertex with minimum distance
int minDistance(int dist[], int visited[], int V) {
    int min = INT_MAX, min_index = -1;
    for (int v = 0; v < V; v++)
        if (!visited[v] && dist[v] <= min) {
            min = dist[v];
            min_index = v;
        }
    return min_index;
}

// Dijkstra algorithm
void dijkstra(struct Graph* g, int src) {
    int V = g->vertices;
    int dist[MAX], visited[MAX];

    for (int i = 0; i < V; i++) {
        dist[i] = INT_MAX;
        visited[i] = 0;
    }
    dist[src] = 0;

    for (int count = 0; count < V - 1; count++) {
        int u = minDistance(dist, visited, V);
        if (u == -1) break;
        visited[u] = 1;

        for (int v = 0; v < V; v++)
            if (!visited[v] && g->adj[u][v] &&
                dist[u] != INT_MAX &&
                dist[u] + g->adj[u][v] < dist[v])
                dist[v] = dist[u] + g->adj[u][v];
    }

    printf("Shortest paths from source %d:\n", src);
    for (int i = 0; i < V; i++) {
        if (dist[i] == INT_MAX)
            printf("To %d: No path\n", i);
        else
            printf("To %d: %d\n", i, dist[i]);
    }
}

int main() {
    FILE *fp = fopen("../data/graph_input.txt", "r");
    if (!fp) {
        printf("Error: graph_input.txt not found!\n");
        return 1;
    }

    int vertices, edges;
    fscanf(fp, "%d %d", &vertices, &edges);

    struct Graph* g = createGraph(vertices);

    int src, dest, weight;
    for (int i = 0; i < edges; i++) {
        fscanf(fp, "%d %d %d", &src, &dest, &weight);
        addEdge(g, src, dest, weight);
    }

    fclose(fp);

    printf("Graph loaded from file successfully!\n");

    int start = 0; // source node
    dijkstra(g, start);

    return 0;
}
