%{

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <inttypes.h>

#include <decimal/decimal>
#include <string>
using namespace std;

#include "bytecode.h"
#include "parser.tab.h"
#include "lex.yy.h"

void yyerror(void* scanner, Zoe::Bytecode& bytecode, const char *s);

%}

%code requires {
#include "bytecode.h"
#include "opcode.h"
}

%define api.pure full
%param { void* scanner }
%parse-param { Zoe::Bytecode& bc }

%defines "lib/parser.tab.h"

%union {
    double number;
    bool   boolean;
    char*  _string;
}

%token <number>  NUMBER
%token <boolean> BOOLEAN
%token <string>  STRING
%token NIL

%left '+' '-'
%left '*' '/' _IDIV '%'
%precedence _NEG
%right _POW

%%

exp: NUMBER             { bc.Add_u8(PUSH_N); bc.Add_f64($1); }
   | BOOLEAN            { bc.Add_u8($1 ? PUSH_Bt : PUSH_Bf); }
   | NIL                { bc.Add_u8(PUSH_Nil); }
   | exp '+' exp        { bc.Add_u8(ADD); }
   | exp '-' exp        { bc.Add_u8(SUB); }
   | exp '*' exp        { bc.Add_u8(MUL); }
   | exp '/' exp        { bc.Add_u8(DIV); }
   | exp _IDIV exp      { bc.Add_u8(IDIV); }
   | exp '%' exp        { bc.Add_u8(MOD); }
   | '-' exp %prec _NEG { bc.Add_u8(NEG); }
   | exp _POW exp       { bc.Add_u8(POW); }
   | '(' exp ')'
   ;

%%


int parse(Zoe::Bytecode& bc, string const& code)
{
    // bytecode header
    bc.Add_u8(0xB4);
    bc.Add_u8(0x7E);
    bc.Add_u8(0xC0);
    bc.Add_u8(0xDE);

    // parse code
    void *scanner;
    yylex_init_extra(&bc, &scanner);
    yy_scan_bytes(code.c_str(), code.size(), scanner);
    int r = yyparse(scanner, bc);
    yylex_destroy(scanner);
    return r;
}


void yyerror(void* scanner, struct Zoe::Bytecode& bc, const char *s)
{
	fprintf(stderr, "%s\n", s);
}

// vim: ts=4:sw=4:sts=4:expandtab
