%{

#include <stdlib.h>

#include "bytecode.h"
#include "parser.tab.h"

%}

%option noyywrap
%option reentrant
%option bison-bridge
%option extra-type="Bytecode*"

NUMBER  -?(0|[1-9][0-9]*)

%%

{NUMBER}        { yylval->integer = strtoll(yytext, NULL, 10); return INTEGER; }
\n              ;
.               return 1;

%%

// vim: ts=4:sw=4:sts=4:expandtab
