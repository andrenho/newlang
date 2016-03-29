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

%nonassoc _LTE _GTE '<' '>' _EQ _NEQ
%left '&' '^' '|'
%left _SHL _SHR
%left '+' '-'
%left '*' '/' _IDIV '%'
%right _POW
%precedence _NOT
%precedence _NEG

%%

exp: NUMBER             { bytecode_addcode(b, PUSH_N); bytecode_addcodef64(b, $1); }
   | BOOLEAN            { bytecode_addcode(b, $1 ? PUSH_Bt : PUSH_Bf); }
   | NIL                { bytecode_addcode(b, PUSH_Nil); }
   | exp _EQ exp        { bytecode_addcode(b, EQ); }
   | exp _NEQ exp       { bytecode_addcode(b, EQ); bytecode_addcode(b, NOT); }
   | exp _LTE exp       { bytecode_addcode(b, LTE); }
   | exp '<' exp        { bytecode_addcode(b, LT); }
   | exp _GTE exp       { bytecode_addcode(b, GTE); }
   | exp '>' exp        { bytecode_addcode(b, GT); }
   | exp '&' exp        { bytecode_addcode(b, AND); }
   | exp '^' exp        { bytecode_addcode(b, XOR); }
   | exp '|' exp        { bytecode_addcode(b, OR); }
   | exp _SHL exp       { bytecode_addcode(b, SHL); }
   | exp _SHR exp       { bytecode_addcode(b, SHR); }
   | exp '+' exp        { bytecode_addcode(b, ADD); }
   | exp '-' exp        { bytecode_addcode(b, SUB); }
   | exp '*' exp        { bytecode_addcode(b, MUL); }
   | exp '/' exp        { bytecode_addcode(b, DIV); }
   | exp _IDIV exp      { bytecode_addcode(b, IDIV); }
   | exp '%' exp        { bytecode_addcode(b, MOD); }
   | exp _POW exp       { bytecode_addcode(b, POW); }
   | '~' exp %prec _NOT { bytecode_addcode(b, NOT); }
   | '-' exp %prec _NEG { bytecode_addcode(b, NEG); }
   | '(' exp ')'
   ;

%%


int parse(Bytecode* b, const char* code)
{
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
