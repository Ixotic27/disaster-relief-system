#include "../include/hashmap.h"
#include <stdlib.h>
#include <string.h>

// ===== Simple DJB2 string hash =====
static unsigned long hash_str(const char *s)
{
    unsigned long h = 5381;
    int c;
    while ((c = *s++))
        h = ((h << 5) + h) + c; //h = h*32+h = h * 33 + c
    return h;
}

// ===== Create a hashmap =====
HashMap *hm_create(int cap)
{
    HashMap *hm = malloc(sizeof(HashMap));
    hm->cap = cap;
    hm->entries = calloc(cap, sizeof(HashEntry)); // initialize entries to NULL
    return hm;
}

// ===== Insert or update a value =====
int hm_put(HashMap *hm, const char *key, Resource *val)
{
    unsigned long h = hash_str(key) % hm->cap;
    int i = h;

    for (;;)
    {
        if (hm->entries[i].key == NULL)
        {
            // Empty slot: insert here
            hm->entries[i].key = strdup(key);
            hm->entries[i].val = val;
            return 0;
        }

        if (strcmp(hm->entries[i].key, key) == 0)
        {
            // Key exists: update value
            hm->entries[i].val = val;
            return 0;
        }

        i = (i + 1) % hm->cap; // linear probing
        if (i == h) return 1;  // hashmap full
    }
}

// ===== Retrieve a value by key =====
Resource *hm_get(HashMap *hm, const char *key)
{
    unsigned long h = hash_str(key) % hm->cap;
    int i = h;

    for (;;)
    {
        if (hm->entries[i].key == NULL) return NULL; // key not found
        if (strcmp(hm->entries[i].key, key) == 0)
            return hm->entries[i].val;

        i = (i + 1) % hm->cap;
        if (i == h) return NULL; // traversed full table
    }
}

// ===== Free hashmap memory =====
void hm_free(HashMap *hm)
{
    if (!hm) return;
    for (int i = 0; i < hm->cap; i++)
        if (hm->entries[i].key)
            free(hm->entries[i].key);
    free(hm->entries);
    free(hm);
}
