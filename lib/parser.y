%{

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "bytecode.h"
#include "parser.tab.h"
#include "lex.yy.h"

void yyerror(void* scanner, Bytecode* bc, const char *s);

%}

%code requires {
#include "bytecode.h"
#include "opcode.h"
}

%define api.pure full
%param { void* scanner }
%parse-param { Bytecode* b }

%defines "lib/parser.tab.h"

%union {
    double number;
    bool   boolean;
    char*  _string;
}

%token <number>  NUMBER
%token <boolean> BOOLEAN
%token <string>  STRING
%token NIL

%left '+' '-'
%left '*' '/' _IDIV '%'
%precedence _NEG
%right _POW

%%

exp: NUMBER             { bytecode_add(b, PUSH_N); bytecode_addf64(b, $1); }
   | BOOLEAN            { bytecode_add(b, $1 ? PUSH_Bt : PUSH_Bf); }
   | NIL                { bytecode_add(b, PUSH_Nil); }
   | exp '+' exp        { bytecode_add(b, ADD); }
   | exp '-' exp        { bytecode_add(b, SUB); }
   | exp '*' exp        { bytecode_add(b, MUL); }
   | exp '/' exp        { bytecode_add(b, DIV); }
   | exp _IDIV exp      { bytecode_add(b, IDIV); }
   | exp '%' exp        { bytecode_add(b, MOD); }
   | '-' exp %prec _NEG { bytecode_add(b, NEG); }
   | exp _POW exp       { bytecode_add(b, POW); }
   | '(' exp ')'
   ;

%%


int parse(Bytecode* b, char* code)
{
    // bytecode header
    bytecode_add(b, 0xB4);
    bytecode_add(b, 0x7E);
    bytecode_add(b, 0xC0);
    bytecode_add(b, 0xDE);

    // parse code
    void *scanner;
    yylex_init_extra(b, &scanner);
    yy_scan_bytes(code, strlen(code), scanner);
    int r = yyparse(scanner, b);
    yylex_destroy(scanner);
    return r;
}


void yyerror(void* scanner, Bytecode* bc, const char *s)
{
	fprintf(stderr, "%s\n", s);
}

// vim: ts=4:sw=4:sts=4:expandtab
