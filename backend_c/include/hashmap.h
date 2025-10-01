#ifndef HASHMAP_H
#define HASHMAP_H
#include "types.h"
typedef struct
{
    char *key;
    Resource *val;
} HMEntry;
typedef struct
{
    HMEntry *entries;
    int cap;
} HashMap;
HashMap *hm_create(int cap);
int hm_put(HashMap *hm, const char *key, Resource *val);
Resource *hm_get(HashMap *hm, const char *key);
void hm_free(HashMap *hm);
#endif
