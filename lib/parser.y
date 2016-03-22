/* Parser to convert "C" assignments to lisp. */
%{
#include <stdio.h>
%}
%locations
%output  "lib/parser.c"
%defines "lib/parser.h"
%pure-parser
%parse-param { void* scanner }
%union {
 int num;
 char* str;
}
%token <str> STRING
%token <num> NUMBER
%%
assignment:
 STRING '=' NUMBER ';' {
     printf( "(setf %s %d)", $1, $3 );
}
;

// vim: ts=4:sw=4:sts=4:expandtab
