%{

#include <math.h>
#include <stdio.h>

#include "parser.tab.h"
#include "lexer.h"

void yyerror(void* scanner, const char *s);

%}

%define api.pure full

%lex-param { void* scanner }
%parse-param { void* scanner }

%union {
	double d;
}

%token <d> NUM

%%

exp: %empty
   | exp NUM		{ printf("(%f)\n", $2); }
   ;

%%

#include <ctype.h>


int main(void)
{
	void* scanner;
	yylex_init(&scanner);
	int r = yyparse(scanner);
	yylex_destroy(scanner);
	return r;
}


void yyerror(void* scanner, const char *s)
{
	fprintf(stderr, "%s\n", s);
}
