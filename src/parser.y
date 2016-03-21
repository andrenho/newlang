%{

#include "lexer.h"

%}

%output  "src/parser.c"
%defines "src/parser.h"

%union {
    int value;
}

%token <value> NUM

%%

expr: NUM { printf("%d\n", $1); }

%%

/* 
vim: ts=4:sw=4:sts=4:expandtab
*/
