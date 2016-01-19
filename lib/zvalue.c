#include "zvalue.h"

#include <assert.h>
#include <stdlib.h>

void zvalue_free_data(UserFunctions* uf, ZValue v)
{
    switch(v.type) {
        case FUNCTION:
            if(v.function.type == BYTECODE) {
                assert(v.function.bfunction.bytecode);
                uf->free(v.function.bfunction.bytecode);
            }
            break;
        case STRING:
            uf->free(v.string);
            break;
        case ARRAY:
            for(size_t i=0; i<v.array.n; ++i) {
                zvalue_free_data(uf, v.array.items[i]);
            }
            uf->free(v.array.items);
            break;
        case INVALID:
            uf->error("Invalid type.");
            break;
        case NIL:
        case BOOLEAN:
        case NUMBER:
            // do nothing, makes gcc happy
            break;
    }

}


// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
