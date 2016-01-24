#ifndef LIB_HASH_H_
#define LIB_HASH_H_

#include <stdbool.h>

#include "lib/zoe.h"
#include "lib/zvalue.h"

typedef struct Hash Hash;

// CONSTRUCTOR/DESTRUCTOR
Hash* hash_new(Zoe* Z);
void  hash_free(Hash* h);

// ACCESS
void    hash_set(Hash* h, ZValue* key, ZValue* value);
ZValue* hash_get(Hash* h, ZValue* key);
bool    hash_del(Hash* h, ZValue* key);

// INFORMATION
double   hash_usage(Hash* h);
uint64_t hash_items(Hash* h);
uint64_t hash_buckets(Hash* h);

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
