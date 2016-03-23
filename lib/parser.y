%{

#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

#include "parser.tab.h"
#include "lexer.h"

void yyerror(void* scanner, Bytecode* bytecode, const char *s);

%}

%code requires {
#include "bytecode.h"
}

%define api.pure full
%param { void* scanner }
%parse-param { Bytecode* bc }

%union {
    int64_t integer;
}

%token <integer> INTEGER;

%%

exp: INTEGER            { printf("%" PRId64 "\n", $1); }
   ;

%%


void parse(Bytecode* bc)
{
    void *scanner;
    yylex_init_extra(bc, &scanner);
    int r = yyparse(scanner, bc);
    yylex_destroy(scanner);
    return r;
}


void yyerror(void* scanner, struct Bytecode* pb, const char *s)
{
	fprintf(stderr, "%s\n", s);
}

// vim: ts=4:sw=4:sts=4:expandtab
