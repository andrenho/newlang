%{

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <inttypes.h>

#include <string>
using namespace std;

#include "bytecode.h"
#include "parser.h"
#include "lexer.h"

void yyerror(void* scanner, Bytecode& bc, const char *s);

%}

%output "lib/parser.cc"

%code requires {
#include "bytecode.h"
#include "opcode.h"

typedef struct String {
    char*  str;
    size_t sz;
    size_t alloc;
} String;

}

%define api.pure full
%param { void* scanner }
%parse-param { Bytecode& b }

%verbose
%printer { fprintf(yyoutput, "%f", $$); } NUMBER;
%printer { fprintf(yyoutput, "%s", $$ ? "true" : "false"); } BOOLEAN;
%printer { fprintf(yyoutput, "%s", $$->c_str()); } STRING;

%defines "lib/parser.h"

%union {
    double       number;
    bool         boolean;
    std::string* str;
    Label        label;
}

%token <number>  NUMBER
%token <boolean> BOOLEAN
%token <str>     STRING IDENTIFIER
%token NIL LET

%token SEP
%precedence '='
%precedence '?'
%precedence ':'
%left       CCAND CCOR
%nonassoc   _LTE _GTE '<' '>' _EQ _NEQ
%left       '&' '^' '|'
%left       _SHL _SHR
%left       '+' '-'
%left       '*' '/' _IDIV '%'
%right      _POW
%precedence _LEN
%left       CONCAT
%precedence _BNOT
%precedence _NOT
%precedence _NEG
%precedence ISNIL
%precedence '['
%precedence '.'


%error-verbose
%start code

/* %expect 1  /* TODO */

%%

code: optsep exps optsep
    | SEP
    ;

optsep: %empty
      | SEP
      ;

exps: exps SEP { b.Add(POP); } exp
    | { b.Add(POP); } exp
    ;

exp: NUMBER             { b.Add(PUSH_N); b.AddF64($1); }
   | BOOLEAN            { b.Add($1 ? PUSH_Bt : PUSH_Bf); }
   | NIL                { b.Add(PUSH_Nil); }
   | IDENTIFIER         { b.AddVariable(*$1); /* free($1); */ }
   | strings
   | array
   | table
   | local_assignment
   | ternary
   | ccand
   | ccor
   | exp _EQ exp         { b.Add(EQ);   }
   | exp _NEQ exp        { b.Add(EQ); 
                           b.Add(BNOT); }
   | exp _LTE exp        { b.Add(LTE);  }
   | exp '<' exp         { b.Add(LT);   }
   | exp _GTE exp        { b.Add(GTE);  }
   | exp '>' exp         { b.Add(GT);   }
   | exp '&' exp         { b.Add(AND);  }
   | exp '^' exp         { b.Add(XOR);  }
   | exp '|' exp         { b.Add(OR);   }
   | exp _SHL exp        { b.Add(SHL);  }
   | exp _SHR exp        { b.Add(SHR);  }
   | exp '+' exp         { b.Add(ADD);  }
   | exp '-' exp         { b.Add(SUB);  }
   | exp '*' exp         { b.Add(MUL);  }
   | exp '/' exp         { b.Add(DIV);  }
   | exp _IDIV exp       { b.Add(IDIV); }
   | exp '%' exp         { b.Add(MOD);  }
   | exp _POW exp        { b.Add(POW);  }
   | '#' exp %prec _LEN  { b.Add(LEN);  }
   | exp CONCAT exp      { b.Add(CAT);  }
   | '!' exp %prec _BNOT { b.Add(BNOT);  }
   | '~' exp %prec _NOT  { b.Add(NOT);  }
   | '-' exp %prec _NEG  { b.Add(NEG);  }
   | exp '[' exp ']'     { b.Add(LOOKUP);  }
   | exp '[' lookup_pos ']'
   | '?' exp %prec ISNIL { b.Add(PUSH_Nil); b.Add(EQ); }
   | exp '.' IDENTIFIER  { b.Add(PUSH_S);
                           b.AddString(*$3);
                           b.Add(LOOKUP); }
   | '(' exp ')'
   | '{' { b.PushScope(); b.Add(PUSH_Nil); } code '}' { b.PopScope(); }
   | '{' { b.PushScope(); b.Add(PUSH_Nil); } '}' { b.PopScope(); }
   ;

// local variable assingment (TODO)
local_assignment: LET assignments
                ;

assignments: one_assignment
           | assignments ',' { b.Add(POP); } one_assignment
           ;

one_assignment: IDENTIFIER '=' exp { 
                    b.VariableAssignment(*$1, false);
                    b.Add(ADDCNST); 
                }
              | '[' { b.MultivarReset(); } mult_identifiers ']' '=' exp {
                    b.AddMultivarAssignment(false);
                    b.Add(ADDMCNST); 
                    b.AddMultivarCounter();
                }
              ;

mult_identifiers: mult_identifiers ',' single_identifier
                | single_identifier
                ;

single_identifier: IDENTIFIER { b.MultivarCreate(*$1); }
                 ;

// strings
string: STRING { b.Add(PUSH_S); b.AddString(*$1); }
      ;

strings: string
       | string strings { b.Add(CAT); }
       ;

// arrays
array: '[' { b.Add(PUSHARY); } array_items ']'
     ;

array_items: %empty
           | array_item
           | array_item ',' array_items
           ;

array_item: exp { b.Add(APPEND); }
          ;

// table initialization
table: '%' '{' { b.Add(PUSHTBL); } tbl_items '}'
     ;

tbl_items: %empty
         | tbl_item
         | tbl_item ',' tbl_items
         ;

tbl_item: IDENTIFIER { 
              b.Add(PUSH_S);
              b.AddString(*$1);
           } ':' exp { b.Add(TBLSET); }
        | '[' exp ']' ':' exp { b.Add(TBLSET); }
        ;

// lookup position
lookup_pos: exp ':' exp  { b.Add(SLICE); }
          | ':'          { b.Add(PUSH_N); b.AddF64(0); } 
            exp          { b.Add(SLICE); }
          | exp ':'      { b.Add(PUSH_Nil); 
                           b.Add(SLICE); }
          | ':'          { b.Add(PUSH_N); b.AddF64(0); 
                           b.Add(PUSH_Nil);
                           b.Add(SLICE); }
          ;

// short-circuit AND
ccand: exp CCAND { 
             /* Code:
                        [exp1]
                        Bfalse is_false
                        [exp2]
                        Bfalse is_false
                        PUSH_Bt
                        JMP is_true
             is_false:	PUSH_Bf
             is_true:   ...
            */
             $<label>2 = b.CreateLabel();
             b.Add(Bfalse); b.AddLabel($<label>2);
         } exp {
             b.Add(Bfalse); b.AddLabel($<label>2);
             b.Add(PUSH_Bt);

             Label is_true = b.CreateLabel();
             b.Add(JMP); b.AddLabel(is_true);

             b.SetLabel($<label>2); 
             b.Add(PUSH_Bf);

             b.SetLabel(is_true); 
         }
     ;

// short-circuit OR
ccor: exp CCOR {
             /* Code:
                        [exp1]
                        Btrue is_true
                        [exp2]
                        Btrue is_true
                        PUSH_Bf
                        JMP is_false
             is_true:	PUSH_Bt
             is_false:  ...
            */
             $<label>2 = b.CreateLabel();
             b.Add(Btrue); b.AddLabel($<label>2);
         } exp {
             b.Add(Btrue); b.AddLabel($<label>2);
             b.Add(PUSH_Bf);

             Label is_false = b.CreateLabel();
             b.Add(JMP); b.AddLabel(is_false);

             b.SetLabel($<label>2); 
             b.Add(PUSH_Bt);

             b.SetLabel(is_false); 
         }
    ;

// ternary operator
ternary: exp '?' {
             /* Code:
                        [exp1]
                        Bfalse  is_false
                        [exp2]
                        JMP     done
             is_false:  [exp3]
                 done:  ...
             */
             $<label>2 = b.CreateLabel();
             b.Add(Bfalse); b.AddLabel($<label>2);
         } exp ':' {
             $<label>4 = b.CreateLabel();
             b.Add(JMP); b.AddLabel($<label>4);
             
             b.SetLabel($<label>2); 
         } exp {
             b.SetLabel($<label>4); 
         }
       ;

%%


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


void yyerror(void* scanner, Bytecode& bc, const char *s)
{
	fprintf(stderr, "%s\n", s);
}

// vim: ts=4:sw=4:sts=4:expandtab
