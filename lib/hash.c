#include "lib/hash.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "lib/zworld.h"

#define INITIAL_SIZE 8

typedef struct ZValueLL {
    ZValue*          key;
    ZValue*          value;
    struct ZValueLL* next;
} ZValueLL;

typedef struct Hash {
    Zoe*       Z;
    ZValueLL** buckets;
    uint64_t   n_buckets;
    uint64_t   n_items;
} Hash;

// {{{ CONSTRUCTOR/DESTRUCTOR

Hash* 
hash_new(Zoe* Z)
{
    Hash* h = calloc(sizeof(Hash), 1);
    h->Z = Z;
    return h;
}

void  
hash_free(Hash* h)
{
    // clear buckets
    for(uint64_t i=0; i<h->n_buckets; ++i) {
        ZValueLL* place = h->buckets[i];
        while(place) {
            ZValueLL* nxt = place->next;
            zoe_dec_ref(h->Z, place->key);
            zoe_dec_ref(h->Z, place->value);
            free(place);
            place = nxt;
        }
    }
    free(h->buckets);

    free(h);
}

// }}}

// {{{ HASH GROW/SHRINK

inline static void hash_setup_for_first_use(Hash* h)
{
    if(!h->buckets) {
        h->n_buckets = INITIAL_SIZE;
        h->buckets = calloc(sizeof(ZValueLL*), h->n_buckets);
    }
}

static void hash_rehash(Hash* h)
{
    // find all keys/values
    uint64_t k = 0;
    struct { ZValue* key; ZValue* value; }* items = malloc(sizeof(ZValue*) * 2 * h->n_items);

    for(uint64_t i=0; i<h->n_buckets; ++i) {
        ZValueLL* place = h->buckets[i];
        while(place) {
            assert(k < h->n_items);
            items[k].key = place->key;
            items[k++].value = place->value;
            place = place->next;
        }
    }

    // release all current keys/values
    for(uint64_t i=0; i<h->n_buckets; ++i) {
        ZValueLL* place = h->buckets[i];
        while(place) {
            ZValueLL* nxt = place->next;
            free(place);
            place = nxt;
        }
        h->buckets[i] = NULL;
    }

    // readd all keys/values
    for(uint64_t i=0; i<k; ++i) {
        // find key
        uint64_t k2 = zoe_hash_value(h->Z, items[i].key);
        uint64_t bk = k2 % h->n_buckets;

        // create value
        ZValueLL* new;
        new = calloc(sizeof(ZValueLL), 1);
        new->key = items[i].key;
        new->value = items[i].value;

        // add to bucket linked list
        if(!h->buckets[bk]) {
            h->buckets[bk] = new;
        } else {
            ZValueLL* place = h->buckets[bk];
            while(place->next) {
                place = place->next;
            }
            place->next = new;
        }
    }

    free(items);
}

static void hash_adjust_usage(Hash* h)
{
    if(hash_usage(h) > 0.66) {
        h->n_buckets *= 2;
        h->buckets = realloc(h->buckets, sizeof(ZValueLL*) * h->n_buckets);
        memset(&h->buckets[h->n_buckets/2], 0, (h->n_buckets/2) * sizeof(ZValueLL*));
        hash_rehash(h);
    }
}

// }}}

// {{{ HASH ACCESS

void
hash_set(Hash* h, ZValue* key, ZValue* value)
{
    // prepare
    hash_setup_for_first_use(h);
    hash_adjust_usage(h);

    // find key
    uint64_t k = zoe_hash_value(h->Z, key);
    uint64_t bk = k % h->n_buckets;

    // create value
    ZValueLL* new;
    new = calloc(sizeof(ZValueLL), 1);
    new->key = key;
    zoe_inc_ref(h->Z, key);
    new->value = value;
    zoe_inc_ref(h->Z, value);

    // add to bucket linked list
    if(!h->buckets[bk]) {
        h->buckets[bk] = new;
    } else {
        ZValueLL* place = h->buckets[bk];
        while(place->next) {
            place = place->next;
        }
        place->next = new;
    }

    // increase usage
    ++h->n_items;
}


ZValue* hash_get(Hash* h, ZValue* key)
{
    // find key
    uint64_t k = zoe_hash_value(h->Z, key);
    uint64_t bk = k % h->n_buckets;

    // find value
    ZValueLL* place = h->buckets[bk];
    while(place) {
        if(zoe_eq(h->Z, key, place->key)) {
            return place->value;
        }
        place = place->next;
    }
    return NULL;
}


bool
hash_del(Hash* h, ZValue* key)
{
    hash_adjust_usage(h);

    // find key
    uint64_t k = zoe_hash_value(h->Z, key);
    uint64_t bk = k % h->n_buckets;

    // find value
    ZValueLL* place = h->buckets[bk];
    ZValueLL* prev = NULL;
    while(place) {
        if(zoe_eq(h->Z, key, place->key)) {
            zoe_dec_ref(h->Z, place->key);
            zoe_dec_ref(h->Z, place->value);
            ZValueLL* nxt = place->next;

            if(!prev) {  // it's the first one
                h->buckets[bk] = nxt;
            } else {
                prev->next = nxt;
            }
            --h->n_items;

            free(place);
            return true;
        }
        prev = place;
        place = place->next;
    }
    return false;
}

// }}}

// {{{ INFORMATION

double
hash_usage(Hash* h)
{
    return (double)h->n_items / (double)h->n_buckets;
}


uint64_t 
hash_buckets(Hash* h)
{
    return h->n_buckets;
}

uint64_t hash_items(Hash* h)
{
    return h->n_items;
}

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
