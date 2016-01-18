%{

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "bytecode.h"
#include "parser.tab.h"

static char* remove_undersc(char* t);
static char* parse_string(char* t);

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

OPERANDS    [\+\-\*\/%\(\)\^\~\&\|\<\>\:\?]

SEPARATOR   [\n\;]+

SPACES      [ \t]+

%x STR

%%

{INTEGER}       { yylval->number = (double)(strtoll(remove_undersc(yytext), NULL, 10)); return NUMBER; }
{BINARY}        { yylval->number = (double)(strtoll(remove_undersc(&yytext[2]), NULL, 2)); return NUMBER; }
{HEX}           { yylval->number = (double)(strtoll(remove_undersc(&yytext[2]), NULL, 16)); return NUMBER; }
{OCTAL}         { yylval->number = (double)(strtoll(remove_undersc(&yytext[2]), NULL, 8)); return NUMBER; }

{FLOAT}         { yylval->number = strtod(remove_undersc(yytext), NULL); return NUMBER; }

true            { yylval->boolean = true; return BOOLEAN; }
false           { yylval->boolean = false; return BOOLEAN; }

'               BEGIN(STR);
<STR>[^']*      { yylval->_string = strdup(parse_string(yytext)); return STRING; }
<STR>'          BEGIN(0);

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
\|\|            { return CCOR; }

\.\.            { return CONCAT; }

{SEPARATOR}     { return SEP; }

{SPACES}        ;
.               return 1;

%%
//{STRING}        { yylval->_string = strdup(parse_string(yytext)); return STRING; }


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


static char* parse_string(char* t)
{
    // transform escapes
    int c = 0;
    while(t[c]) {
        if(t[c] == '\\') {
            switch(t[c+1]) {
                case '\'':  // single quotes
                case '\\':  // reverse slash
                    memmove(t+c, t+c+1, strlen(t)-c);  // just remove the slash
                    continue;
                case 'n':  // 'n' (char 13)
                    memmove(t+c, t+c+1, strlen(t)-c);
                    t[c] = 10;
                    continue;
                case 'r':  // 'n' (char 10)
                    memmove(t+c, t+c+1, strlen(t)-c);
                    t[c] = 13;
                    continue;
                case 'x': { // 'x' (hexadecimal follows)
                    char hex[3] = {0,0,0};
                    memcpy(hex, &t[c+2], 2);
                    char *endptr = &hex[2], *orig = &hex[2];
                    uint8_t ch = (uint8_t)strtol(hex, &endptr, 16);
                    if(endptr != orig) {
                        // TODO - what is the better way to fire this error?
                        fprintf(stderr, "Hexadecimal digit expected.\n");
                        yyterminate();
                    }
                    memmove(t+c, t+c+3, strlen(t)-c);
                    t[c] = ch;
                    continue;
                }
                // TODO - unicode - how?
            }
        }
        ++c;
    }

    return t;
}

// vim: ts=4:sw=4:sts=4:expandtab
