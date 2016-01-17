%{

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "bytecode.h"
#include "parser.tab.h"

static char* remove_undersc(char* t);
static char* remove_quotes(char* t);

%}

%option noyywrap
%option reentrant
%option bison-bridge
%option extra-type="Bytecode*"

INTEGER     [0-9][0-9_]*
BINARY      0[Bb][01_]+
HEX         0[Xx][0-9A-Fa-f_]+
OCTAL       0[Oo][0-7_]+

FLOAT       [0-9][0-9_]*\.[0-9_]+

STRING      '(\\.|[^'])*\'

OPERANDS    [\+\-\*\/%\(\)\^\~\&\|\<\>]

SEPARATOR   [\n\;]+

SPACES      [ \t]+

%%

{INTEGER}       { yylval->number = (double)(strtoll(remove_undersc(yytext), NULL, 10)); return NUMBER; }
{BINARY}        { yylval->number = (double)(strtoll(remove_undersc(&yytext[2]), NULL, 2)); return NUMBER; }
{HEX}           { yylval->number = (double)(strtoll(remove_undersc(&yytext[2]), NULL, 16)); return NUMBER; }
{OCTAL}         { yylval->number = (double)(strtoll(remove_undersc(&yytext[2]), NULL, 8)); return NUMBER; }

{FLOAT}         { yylval->number = strtod(remove_undersc(yytext), NULL); return NUMBER; }

{STRING}        { strcpy(yylval->_string, remove_quotes(yytext)); return STRING; }

true            { yylval->boolean = true; return BOOLEAN; }
false           { yylval->boolean = false; return BOOLEAN; }

nil             { return NIL; }

{OPERANDS}      { return yytext[0]; }

\/\/            { return _IDIV; }
\*\*            { return _POW; }
\<\<            { return _SHL; }
\>\>            { return _SHR; }

\>\=            { return _GTE; }
\<\=            { return _LTE; }

\=\=            { return _EQ; }
\!\=            { return _NEQ; }

\&\&            { return CCAND; }

{SEPARATOR}     { return SEP; }

{SPACES}        ;
.               return 1;

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


static char* remove_quotes(char* t)
{
    int len = strlen(t)-2;
    memmove(t, &t[1], strlen(t)-2);
    t[strlen(t)-2] = '\0';

    int c = 0;
    while(t[c]) {
        if(t[c] == '\\' && t[c+1] == '\'') {
            memmove(t+c, t+c+1, strlen(t)-c);
            --len;
            continue;
        }
        ++c;
    }

    return t;
}

// vim: ts=4:sw=4:sts=4:expandtab
