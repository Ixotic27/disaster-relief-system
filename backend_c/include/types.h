#ifndef TYPES_H
#define TYPES_H

#define IDLEN 32
#define NAMELEN 64

typedef struct {
    char id[IDLEN];
    char name[NAMELEN];
    char category[32]; // required by fileio.c
    int quantity;
} Resource;

typedef struct {
    char id[IDLEN];
    char name[NAMELEN];
    int severity;
    int population;    // required by main.c and allocation.c
} Region;

typedef struct {
    char id[IDLEN];
    char region_id[IDLEN];
    char resource_id[IDLEN];
    int qty_needed;
} Request;

#endif // TYPES_H
