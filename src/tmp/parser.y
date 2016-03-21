%{

#include "lexer.h"
#include "compiler.h"
void yyerror(const char* s);

%}

%output  "src/parser.c"
%defines "src/parser.h"

%union {
    int value;
}

%token <value> NUM

%%

expr: %empty
    | NUM { cp_add(PUSH_I, $1); }

%%

/* 
vim: ts=4:sw=4:sts=4:expandtab
*/
