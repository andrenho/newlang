%{

#define YY_HEADER_EXPORT_START_CONDITIONS

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "bytecode.h"
#include "parser.tab.h"

static void str_addstr(String* str, char* s);
static char* remove_undersc(char* t);

%}

%option noyywrap
%option reentrant
%option bison-bridge
%option extra-type="Bytecode*"
%option stack
%option warn

INTEGER     [0-9][0-9_]*
BINARY      0[Bb][01_]+
HEXDIGIT    [0-9A-Fa-f_]
HEX         0[Xx]{HEXDIGIT}+
OCTAL       0[Oo][0-7_]+

FLOAT       [0-9][0-9_]*\.[0-9_]+

OPERANDS    [\+\-\*\/%\(\)\^\~\&\|\<\>\:\?\[\]\,]

SEPARATOR   [\n\;]+

SPACES      [ \t]+

%x STR CMT
%s SB

%%

<CMT,INITIAL>"/*"  { yy_push_state(CMT, yyscanner); }
<CMT>[^\*\/]*      ;
<CMT>"*"+[^\/]     ;
<CMT>\/            ;
<CMT>"*/"          { yy_pop_state(yyscanner); }
"//"+.*\n          ;

{INTEGER}       { yylval->number = (double)(strtoll(remove_undersc(yytext), NULL, 10)); return NUMBER; }
{BINARY}        { yylval->number = (double)(strtoll(remove_undersc(&yytext[2]), NULL, 2)); return NUMBER; }
{HEX}           { yylval->number = (double)(strtoll(remove_undersc(&yytext[2]), NULL, 16)); return NUMBER; }
{OCTAL}         { yylval->number = (double)(strtoll(remove_undersc(&yytext[2]), NULL, 8)); return NUMBER; }

{FLOAT}         { yylval->number = strtod(remove_undersc(yytext), NULL); return NUMBER; }

true            { yylval->boolean = true; return BOOLEAN; }
false           { yylval->boolean = false; return BOOLEAN; }

'                     { yy_push_state(STR, yyscanner);
                        yylval->_string = (String) { .str=calloc(20, 1), .sz = 0, .alloc = 20 }; }
<STR>\\n              { str_addstr(&yylval->_string, "\n"); }
<STR>\\r              { str_addstr(&yylval->_string, "\r"); }
<STR>\\\\             { str_addstr(&yylval->_string, "\\"); }
<STR>\\x{HEXDIGIT}{2} { str_addstr(&yylval->_string, (char[]) { (char)strtol(&yytext[2], NULL, 16), 0 }); }
<STR>\\               { str_addstr(&yylval->_string, "\\"); }
<STR>[^'\\\$]*        { str_addstr(&yylval->_string, yytext); }
<STR>\$'              { str_addstr(&yylval->_string, "$"); yy_pop_state(yyscanner); return STRING; }
<STR>\$[^\{]          { str_addstr(&yylval->_string, yytext); }
<STR>\$\{             { yy_push_state(SB, yyscanner); return STRING; }
<STR>'                { yy_pop_state(yyscanner); return STRING; }

<SB>\}                { yy_pop_state(yyscanner);
                        yylval->_string = (String) { .str=calloc(20, 1), .sz = 0, .alloc = 20 }; }

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


static void str_addstr(String* str, char* s)
{
    size_t len = strlen(s);
    size_t new_size = str->sz + len + 1;
    if(new_size > str->alloc) {
        size_t new_alloc = (new_size > (str->alloc * 2)) ? new_size : (str->alloc * 2);
        str->str = realloc(str->str, new_alloc);
    }
    //strcat(str->str, s);
    memcpy(&str->str[strlen(str->str)], s, len);
}


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
