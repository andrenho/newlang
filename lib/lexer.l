%{

#include <stdlib.h>
#include <string.h>

#include "bytecode.h"
#include "parser.tab.hh"

static char* remove_undersc(char* t);

%}

%option noyywrap
%option reentrant
%option bison-bridge
%option extra-type="Zoe::Bytecode*"

INTEGER     [0-9][0-9_]*
BINARY      0[Bb][01_]+
HEX         0[Xx][0-9A-Fa-f_]+
OCTAL       0[Oo][0-7_]+

%%

{INTEGER}       { yylval->integer = strtoll(remove_undersc(yytext), NULL, 10); return INTEGER; }
{BINARY}        { yylval->integer = strtoll(remove_undersc(&yytext[2]), NULL, 2); return INTEGER; }
{HEX}           { yylval->integer = strtoll(remove_undersc(&yytext[2]), NULL, 16); return INTEGER; }
{OCTAL}         { yylval->integer = strtoll(remove_undersc(&yytext[2]), NULL, 8); return INTEGER; }
\n              ;
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

// vim: ts=4:sw=4:sts=4:expandtab
