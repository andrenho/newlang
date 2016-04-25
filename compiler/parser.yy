%{

// ignore warnings from G++
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"

#include "compiler/parser.hh"
#include "compiler/lexer.hh"

void yyerror(void* scanner, const char *s);

%}

/* make the perser reentrant */
%define api.pure full
%param { void* scanner }

%union {
    int x; // TODO
}

%%

exp: %empty
   ;

%%

void yyerror(void* scanner, const char* s)
{
}

// vim: ts=4:sw=4:sts=4:expandtab:syntax=yacc
