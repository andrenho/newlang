%{

#include <math.h>
#include <stdio.h>

#include "parser.tab.h"
#include "lexer.h"

void yyerror(void* scanner, struct PassToBison* pb, const char *s);

%}

%code requires {
struct PassToBison {
	void* scanner;
};

}

%define api.pure full

%param { void* scanner }
%parse-param { struct PassToBison * pb }

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
	struct PassToBison pb;

	yylex_init_extra(&pb, &scanner);
	int r = yyparse(scanner, &pb);
	yylex_destroy(scanner);
	return r;
}


void yyerror(void* scanner, struct PassToBison* pb, const char *s)
{
	fprintf(stderr, "%s\n", s);
}
