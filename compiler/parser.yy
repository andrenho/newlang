%{

// ignore warnings from G++
#if !__llvm__  // warnings not supported by clang++
#  pragma GCC diagnostic ignored "-Wuseless-cast"
#  pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#else
#  pragma clang diagnostic ignored "-Wunreachable-code"
#endif
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wmissing-declarations"

#include <cmath>
#include <cstdint>

#include <limits>
#include <iostream>
using namespace std;

#include "compiler/parser.hh"
#include "compiler/lexer.hh"
#include "vm/exceptions.hh"

/* 
 * PROTOTYPES
 */
static void add_number(Bytecode& b, double num);
static void add_variables(Bytecode& b, vector<string> const& names, bool mut);
void yyerror(YYLTYPE* yylloc, void* scanner, Bytecode& b, const char *s) __attribute__((noreturn));

%}

/* make the perser reentrant */
%define api.pure full
%locations

/* create debugging information *
%define parse.trace true

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
%code requires {
#include <vector>
#include <string>
using namespace std;
#include "compiler/bytecode.hh"
}

%union {
    double       number;
    bool         boolean;
    size_t       integer;
    uint8_t      u8;
    std::string* str;
    std::vector<string>* vec;
    Label        label;
}

%token <number>  NUMBER
%token <boolean> BOOLEAN
%token <str>     STRING IDENTIFIER
%token NIL SEP _MUT _PUB _DEL LET FN

%type <boolean> mut_opt
%type <str> string strings
%type <vec> varnames
%type <integer> array_items table_items table_items_x     /* $$ is a counter */
%type <u8> properties                                     /* $$ is a TableConfig instance */

%nonassoc '='
%precedence '['
%left '.'
%precedence '('  /* lowest? */

%%

/* TODO - there are several points in this parser that I'm catching zoe_syntax_error exceptions,
 *        just to call yyerror, where they are rethrown. The pourpose of this is to be able
 *        to free the scanner object. There's gotta be a better way to do this - like a generic
 *        catch around `yyparse`. But, right now, I tested this and I'm getting segmentation
 *        faults. */

//
// CODE
//
code: optsep exps optsep
    | SEP
    ;

optsep: %empty
      | SEP
      ;

exps: exps SEP { b.Add(POP); } exp
    |          { b.Add(POP); } exp
    ;


//
// EXPRESSIONS
//
exp: literal_exp
   | array_init
   | table_init
   | var_init
   | var_assign
   | var_get
   | set_op
   | get_op
   | function_def
   | exp function_call
   | block
   ;

block: '{' { b.PushScope(); b.Add(PSHS); b.Add(PNIL); } code '}' { b.Add(POPS); b.PopScope(); }
     | '{' '}'
     ;


// 
// LITERAL EXPRESSIONS
//
literal_exp: NIL        { b.Add(PNIL); }
           | NUMBER     { add_number(b, $1); }
           | BOOLEAN    { b.Add($1 ? PBT : PBF); }
           | strings    { b.Add(PSTR, *$1); delete $1; }
           ;

string: STRING
      ;

strings: string             { $$ = new string(*$1); delete $1; }
       | string strings     { $$ = new string(*$1 + *$2); delete $1; delete $2; }
       ;

//
// VARIABLE MANAGEMENT
//
var_get: IDENTIFIER { 
            string id = *$1; delete $1;
            try {
                b.Add(GVAR, b.GetVariableIndex(id, nullptr));
            } catch(zoe_syntax_error const& e) {
                yyerror(&yylloc, scanner, b, e.what());
            }
        }
        ;

var_init: LET mut_opt IDENTIFIER {
                string s = *$3; delete $3; 
                try {
                    b.CreateVariable(s, $mut_opt);
                } catch(zoe_syntax_error const& e) {
                    yyerror(&yylloc, scanner, b, e.what());
                }
                b.Add(PNIL); 
                b.Add(CVAR); 
            }
        | LET mut_opt IDENTIFIER '=' exp { 
                string s = *$3; delete $3; 
                try {
                    b.CreateVariable(s, $mut_opt);
                } catch(zoe_syntax_error const& e) {
                    yyerror(&yylloc, scanner, b, e.what());
                }
                b.Add(CVAR); 
            }
        | LET mut_opt '[' varnames ']' '=' exp { 
                for(auto it = rbegin(*$varnames); it != rend(*$varnames); ++it) {
                    try {
                        b.CreateVariable(*it, $mut_opt);
                    } catch(zoe_syntax_error const& e) {
                        delete $varnames;
                        yyerror(&yylloc, scanner, b, e.what());
                    }
                }
                b.Add(CMVAR, static_cast<uint16_t>($varnames->size()));
                delete $varnames;
            }
        ;

var_assign: IDENTIFIER '=' exp  { 
                bool mut;
                string s = *$1; delete $1;
                uint32_t n;
                try {
                    n = b.GetVariableIndex(s, &mut);
                } catch(zoe_syntax_error const& e) {
                    yyerror(&yylloc, scanner, b, e.what());
                }
                if(!mut) {
                    yyerror(&yylloc, scanner, b, ("Variable '" + s + "' is not mutable.").c_str());
                    return 1;
                }
                b.Add(SVAR, n);
            }
          ;

mut_opt: %empty  { $$ = false; }
       | _MUT    { $$ = true;  }
       ;

varnames: IDENTIFIER                { $$ = new vector<string>({*$1}); delete $1; }
        | varnames ',' IDENTIFIER   { $$->push_back(*$3); delete $3; }
        ;

// 
// ARRAY INITIALIZATION
//
array_init: '[' array_items opt_comma ']' { b.Add(PARY, static_cast<uint16_t>($2)); }
          ;

opt_comma: %empty
         | ','
         ;

array_items: %empty              { $$ = 0; }
           | exp                 { $$ = 1; }
           | array_items ',' exp { ++$$; }
           ;

//
// TABLE INITIALIZATION
//
table_init: '%' opt_identifier '{' table_items opt_comma '}' 
              { b.Add(PTBL, static_cast<uint16_t>($table_items)); }
          | '&' opt_identifier '{' table_items_x opt_comma '}'
              { b.Add(PTBX, static_cast<uint16_t>($table_items_x)); }
          ;

opt_identifier: %empty     { b.Add(PNIL); }
              | '[' exp ']'
              ;

table_items: %empty                      { $$ = 0; }
           | table_item                  { $$ = 1; }
           | table_items ',' table_item  { ++$$; }
           ;

table_item: properties IDENTIFIER { b.Add(PSTR, *$2); delete $2; b.Add(PN8, $1); } ':' exp
          | properties '[' exp ']' { b.Add(PN8, $1); } ':' exp  
          ;

properties: %empty          { $$ = 0; }
          | _MUT properties { $$ = MUT | $2; }
          | _PUB properties { $$ = PUB | $2; }
          ;

table_items_x: %empty                          { $$ = 0; }
             | table_item_x                    { $$ = 1; }
             | table_items_x ',' table_item_x  { ++$$; }
             ;

table_item_x: IDENTIFIER { b.Add(PSTR, *$1); delete $1; } ':' exp
            | '[' exp ']' ':' exp  
            ;

// 
// TABLE EXPRESSION
//
table_exp: exp '.' IDENTIFIER { b.Add(PSTR, *$3); delete $3; }
         | exp '[' exp ']'
         ;

// 
// SET OPERATOR
//
set_op: table_exp '=' exp { b.Add(SET, static_cast<uint8_t>(PUB|MUT)); }
      ;

// 
// GET OPERATOR
//
get_op: table_exp { b.Add(GET, 0, 0); }
      ;

// 
// FUNCTION DEFINITION
//
function_def: FN '(' function_pars ')' block
            ;

function_pars: %empty
             ;

//
// FUNCTION CALLS
//
function_call: '(' call_pars ')'
             ;

call_pars: %empty
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


void yyerror(YYLTYPE* yylloc, void* scanner, Bytecode& b, const char* s)
{
    yylex_destroy(scanner);     // gives us a clean exit
    throw zoe_syntax_error(s);
    //cerr << "error in " << yylloc->first_line << ":" << yylloc->first_column << ": " << s << "\n";
}

// vim: ts=4:sw=4:sts=4:expandtab:syntax=yacc
