/* Parser to convert "C" assignments to lisp. */
%{
/* Pass the argument to yyparse through to yylex. */
#define YYPARSE_PARAM scanner
#define YYLEX_PARAM   scanner
%}
%locations
%output  "lib/parser.c"
%defines "lib/parser.h"
%pure-parser
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
