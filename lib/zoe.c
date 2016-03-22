#include "zoe.h"

#include <stdlib.h>
#include <stdio.h>

Zoe* 
zoe_new(void)
{
    void* sc;
    yylex_init(&sc);
    
    //yylex
    //int state = yy_scan_string("a=3;", sc);

    //yyparse(sc);
    yylex_destroy(sc);

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
