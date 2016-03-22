%{

#include "parser.tab.h"

%}

%option noyywrap
%option reentrant
%option bison-bridge

%%

[0-9]+	{ yylval->d = strtod(yytext, NULL); return NUM; }
\n	;
.	return 1;

%%

/*
int main() {
	yyscan_t scanner;
 
	struct Data data = { 0, 0 };

	yylex_init_extra(&data, &scanner);
	yylex(scanner);
	yylex_destroy(scanner);

	printf("a = %d, b = %d\n", data.a, data.b);

	return 0;
}
*/
