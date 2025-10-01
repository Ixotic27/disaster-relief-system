#include "../include/hashmap.h"
#include <stdlib.h>
#include <string.h>
static unsigned long hash_str(const char *s)
{
    unsigned long h = 5381;
    int c;
    while ((c = *s++))
        h = ((h << 5) + h) + c;
    return h;
}
HashMap *hm_create(int cap)
{
    HashMap *hm = malloc(sizeof(HashMap));
    hm->cap = cap;
    hm->entries = calloc(cap, sizeof(HMEntry));
    return hm;
}
int hm_put(HashMap *hm, const char *key, Resource *val)
{
    unsigned long h = hash_str(key) % hm->cap;
    int i = h;
    for (;;)
    {
        if (hm->entries[i].key == NULL)
        {
            hm->entries[i].key = strdup(key);
            hm->entries[i].val = val;
            return 0;
        }
        if (strcmp(hm->entries[i].key, key) == 0)
        {
            hm->entries[i].val = val;
            return 0;
        }
        i = (i + 1) % hm->cap;
        if (i == h)
            return 1;
    }
}
Resource *hm_get(HashMap *hm, const char *key)
{
    unsigned long h = hash_str(key) % hm->cap;
    int i = h;
    for (;;)
    {
        if (hm->entries[i].key == NULL)
            return NULL;
        if (strcmp(hm->entries[i].key, key) == 0)
            return hm->entries[i].val;
        i = (i + 1) % hm->cap;
        if (i == h)
            return NULL;
    }
}
void hm_free(HashMap *hm)
{
    for (int i = 0; i < hm->cap; i++)
        if (hm->entries[i].key)
            free(hm->entries[i].key);
    free(hm->entries);
    free(hm);
}
