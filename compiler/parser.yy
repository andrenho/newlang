%{

// ignore warnings from G++
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wmissing-declarations"

#include <cmath>
#include <cstdint>

#include <limits>
#include <iostream>
using namespace std;

#include "compiler/bytecode.hh"
#include "compiler/parser.hh"
#include "compiler/lexer.hh"

/* 
 * PROTOTYPES
 */
static void add_number(Bytecode& b, double num);
void yyerror(void* scanner, Bytecode& b, const char *s);

%}

/* make the perser reentrant */
%define api.pure full

/* since we are operating reentrant, we need to pass the scanner state around */
%param { void* scanner }

/* we need to pass a Bytecode object around to fill it */
%parse-param { Bytecode& b }

/* define debugging output */
%verbose
%error-verbose
%printer { fprintf(yyoutput, "%f", $$); } NUMBER;
%printer { fprintf(yyoutput, "%s", $$ ? "true" : "false"); } BOOLEAN;
%printer { fprintf(yyoutput, "'%s'", $$->c_str()); } STRING;
%printer { fprintf(yyoutput, "%s", $$->c_str()); } IDENTIFIER;

/* start parsing by this token */
%start code

/*
 * DATA STRUCTURE
 */
/*
%union {
    double       number;
    bool         boolean;
    std::string* str;
    Label        label;
}
*/
%define api.value.type union
%token <double>       NUMBER
%token <bool>         BOOLEAN
%token <std::string*> STRING IDENTIFIER
%token NIL LET SEP

%%

code: optsep exps optsep
    | SEP
    ;

optsep: %empty
      | SEP
      ;

exps: exps SEP { b.Add(POP); } exp
    |          { b.Add(POP); } exp
    ;

exp: NIL        { b.Add(PNIL); }
   | NUMBER     { add_number(b, $1); }
   | BOOLEAN    { b.Add($1 ? PBT : PBF); }
   | strings
   ;

// strings
string: STRING  { b.Add(PSTR, *$1); delete $1; }
      ;

strings: string
       | string strings
       ;

%%

static void add_number(Bytecode& b, double num)
{
    double i;
    if(num >= 0 && num <= 255 && fabs(modf(num, &i)) < numeric_limits<double>::epsilon()) {
        b.Add(PN8, static_cast<uint8_t>(i));
    } else {
        b.Add(PNUM, num);
    }
}

int parse(Bytecode& b, string const& code)
{
    // parse code
    void *scanner;
    yylex_init_extra(&b, &scanner);
    yy_scan_bytes(code.c_str(), code.size(), scanner);
    int r = yyparse(scanner, b);
    yylex_destroy(scanner);
    return r;
}


void yyerror(void* scanner, Bytecode& b, const char* s)
{
    cerr << s << "\n";
}

// vim: ts=4:sw=4:sts=4:expandtab:syntax=yacc
