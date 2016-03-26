%{

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <inttypes.h>
#include <string>
using namespace std;

#include "bytecode.h"
#include "parser.tab.hh"
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
    int64_t integer;
}

%token <integer> INTEGER;

%%

exp: %empty
   | INTEGER            { bc.Add_u8(PUSH_I); bc.Add_i64($1); }
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
