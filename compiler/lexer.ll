%{

// ignore warnings from G++
#if !__llvm__  // warnings not supported by clang++
#  pragma GCC diagnostic ignored "-Wuseless-cast"
#  pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#else
#  pragma clang diagnostic ignored "-Wdeprecated-register"
#  pragma clang diagnostic ignored "-Wmissing-noreturn"
#endif
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"

// set start conditions from within bison
#define YY_HEADER_EXPORT_START_CONDITIONS

// initialize locations
#define YY_USER_ACTION yylloc->first_line = yylloc->last_line = yylineno; yylloc->first_column = yylloc->last_column = 0;

#include <cstdlib>
#include <cstring>

#include "compiler/bytecode.hh"
#include "compiler/parser.hh"

static char* remove_undersc(char* t);

%}

/* do not call `yywrap` upon end-of-file */
%option noyywrap

/* make the lexer reentrant */
%option reentrant

/* we are using a bison-compatible scanner */
%option bison-bridge
%option bison-locations

/* we need to pass a Bytecode object around to fill it */
%option extra-type="Bytecode*"

/* activate stack to push and pop out of STR state */
%option stack

/* add warnings */
%option warn

/*
 * EXPRESSIONS
 */

INTEGER      [0-9][0-9_]*
BINARY       0[Bb][01_]+
HEXDIGIT     [0-9A-Fa-f_]
HEX          0[Xx]{HEXDIGIT}+
OCTAL        0[Oo][0-7_]+

FLOAT        [0-9][0-9_]*\.[0-9_]+

IDENTIFIER   [[:alpha:]_][[:alnum:]_]*

SPECIALCHARS [\+\-\*\/%\(\)\^\~\&\|\<\>\:\?\[\]\,\#\!\{\}\.\=]

SEPARATOR    [\n\;]+

SPACES       [ \t]+

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
nil             { return NIL; }
mut             { return _MUT; }
pub             { return _PUB; }
let             { return LET; }
del             { return _DEL; }

'                     { yy_push_state(STR, yyscanner); yylval->str = new string(); }
<STR>\\n              { yylval->str->append(1, '\n'); }
<STR>\\r              { yylval->str->append(1, '\r'); }
<STR>\\\\             { yylval->str->append(1, '\\'); }
<STR>\\x{HEXDIGIT}{2} { yylval->str->append(1, (char)strtol(&yytext[2], NULL, 16)); }
<STR>\\               { yylval->str->append(1, '\\'); }
<STR>[^'\\\$]*        { yylval->str->append(yytext); }
<STR>\$'              { yylval->str->append(1, '$');
                        yy_pop_state(yyscanner); 
                        return STRING; }
<STR>\$[^\{]          { yylval->str->append(yytext); }
<STR>\$\{             { yy_push_state(SB, yyscanner); return STRING; }
<STR>'                { yy_pop_state(yyscanner); return STRING; }

<SB>\}                { yy_pop_state(yyscanner);
                        yylval->str = new string(); }

{SPECIALCHARS}  { return yytext[0]; }

{IDENTIFIER}    { yylval->str = new string(yytext); return IDENTIFIER; }

{SEPARATOR}     { return SEP;    }

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

// vim: ts=4:sw=4:sts=4:expandtab:syntax=lex
