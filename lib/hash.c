#include "lib/hash.h"

#include <stdlib.h>

typedef struct Hash {
    Zoe* Z;
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
    free(h);
}

// }}}

// {{{ HASH ACCESS

void
hash_set(Hash* h, ZValue* key, ZValue* value)
{
}


ZValue* hash_get(Hash* h, ZValue* key)
{
    return NULL;
}


void
hash_del(Hash* h, ZValue* key)
{
}

// }}}


// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
