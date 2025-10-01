#ifndef TYPES_H
#define TYPES_H
#define IDLEN 32
typedef struct
{
    char id[IDLEN];
    char name[64];
    int quantity;
} Resource;
typedef struct
{
    char id[IDLEN];
    char name[64];
    int severity;
} Region;
typedef struct
{
    char id[IDLEN];
    char region_id[IDLEN];
    char resource_id[IDLEN];
    int qty_needed;
} Request;
#endif