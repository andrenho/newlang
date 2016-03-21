#include "zoe.h"

#include <stdlib.h>

Zoe* 
zoe_new(void)
{
    Zoe* zoe = calloc(sizeof(Zoe), 1);
    return zoe;
}


void 
zoe_terminate(Zoe** zoe)
{
    free(*zoe);
    *zoe = NULL;
}


// vim: ts=4:sw=4:sts=4:expandtab
