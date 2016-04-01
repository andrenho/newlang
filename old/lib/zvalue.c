#include "lib/zvalue.h"

#include <assert.h>
#include <stdlib.h>

void zvalue_free_data(ZValue v, ERROR errorf)
{
    switch(v.type) {
        case FUNCTION:
            if(v.function.type == BYTECODE) {
                assert(v.function.bfunction.bytecode);
                free(v.function.bfunction.bytecode);
            }
            break;
        case STRING:
            free(v.string);
            break;
        case ARRAY:
            for(size_t i=0; i<v.array.n; ++i) {
                zvalue_free_data(v.array.items[i], errorf);
            }
            free(v.array.items);
            break;
        case INVALID:
            errorf("Invalid type.");
            break;
        case NIL:
        case BOOLEAN:
        case NUMBER:
            // do nothing, makes gcc happy
            break;
    }

}


// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
