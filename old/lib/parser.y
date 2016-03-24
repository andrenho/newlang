%{

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "parser.tab.h"
#include "lex.yy.h"

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

exp: %empty
   | INTEGER            { bc_push(bc, PUSH_I, $1); }
   ;

%%


int parse(Bytecode* bc, const char* code)
{
    void *scanner;
    yylex_init_extra(bc, &scanner);
    yy_scan_bytes(code, strlen(code), scanner);
    int r = yyparse(scanner, bc);
    yylex_destroy(scanner);
    return r;
}


void yyerror(void* scanner, struct Bytecode* bc, const char *s)
{
	fprintf(stderr, "%s\n", s);
}

// vim: ts=4:sw=4:sts=4:expandtab
