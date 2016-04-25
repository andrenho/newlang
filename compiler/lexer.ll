%{

// ignore warnings from G++
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"

// set start conditions from within bison
#define YY_HEADER_EXPORT_START_CONDITIONS

#include <cstdlib>
#include <cstring>

#include "compiler/bytecode.hh"
#include "compiler/parser.hh"

static char* remove_undersc(char* t);

%}

/* do not call `yywrap` upon end-of-file */
%option noyywrap

/* make the lexer reentrant */
%option reentrant

/* we are using a bison-compatible scanner */
%option bison-bridge

/* we need to pass a Bytecode object around to fill it */
%option extra-type="Bytecode*"

/* activate stack to push and pop out of STR state */
%option stack

/* add warnings */
%option warn

/*
 * EXPRESSIONS
 */

INTEGER      [0-9][0-9_]*

%%

{INTEGER}       { yylval->number = (double)(strtoll(remove_undersc(yytext), NULL, 10)); return NUMBER; }

%%

static char* remove_undersc(char* t)
{
    int c = 0;
    while(t[c]) {
        if(t[c] == '_') {
            memmove(t+c, t+c+1, strlen(t)-c);
            continue;
        }
        ++c;
    }
    return t;
}

// vim: ts=4:sw=4:sts=4:expandtab:syntax=lex
