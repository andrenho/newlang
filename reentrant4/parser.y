%{

#include <math.h>
#include <stdio.h>

#include "parser.tab.h"
#include "lexer.h"

void yyerror(void* scanner, const char *s);

%}

%pure-parser
%lex-param { void* scanner }
%parse-param { void* scanner }

%token NUM

%%

exp: %empty
   | exp NUM		{ printf("(%d)\n", $2); }
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
