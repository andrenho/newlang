%{

#include <stdlib.h>
#include <stdio.h>
#include "parser.h"

%}

%option outfile="src/lexer.c" header-file="src/lexer.h"
%option warn noyywrap yylineno

NUMBER  [1-9][0-9]*

%%

{NUMBER}    { yylval.value = atoi(yytext); return NUM; }
.           ;

%%

int yyerror(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
    return 0;
}

/* 
vim: ts=4:sw=4:sts=4:expandtab
*/
