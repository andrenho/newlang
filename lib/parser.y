%{

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "bytecode.h"
#include "parser.tab.h"
#include "lex.yy.h"

void yyerror(void* scanner, Bytecode* bc, const char *s);

%}

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
%parse-param { Bytecode* b }

%verbose
%printer { fprintf(yyoutput, "%f", $$); } NUMBER;
%printer { fprintf(yyoutput, "%s", $$ ? "true" : "false"); } BOOLEAN;
%printer { fprintf(yyoutput, "%s", $$.str); } STRING

%defines "lib/parser.tab.h"

%union {
    double      number;
    bool        boolean;
    String      _string;
    const char* str;
    Label       label;
}

%token <number>  NUMBER
%token <boolean> BOOLEAN
%token <_string> STRING
%token <str>     IDENTIFIER
%token NIL SEP

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

%%

code: exps             { bytecode_addcode(b, END); }
    ;

exps: %empty
    | exp SEP exps
    | exp
    | SEP               { bytecode_addcode(b, PUSH_Nil); }
    ;

exp: NUMBER             { bytecode_addcode(b, PUSH_N); bytecode_addcodef64(b, $1); }
   | BOOLEAN            { bytecode_addcode(b, $1 ? PUSH_Bt : PUSH_Bf); }
   | NIL                { bytecode_addcode(b, PUSH_Nil); }
   | strings
   | array
   | table
   | exp '.' IDENTIFIER { bytecode_addcode(b, PUSH_S);
                          bytecode_addcodestr(b, $3);
                          bytecode_addcode(b, LOOKUP); }
   | ternary
   | ccand
   | ccor
   | exp _EQ exp         { bytecode_addcode(b, EQ);   }
   | exp _NEQ exp        { bytecode_addcode(b, EQ); 
                           bytecode_addcode(b, BNOT); }
   | exp _LTE exp        { bytecode_addcode(b, LTE);  }
   | exp '<' exp         { bytecode_addcode(b, LT);   }
   | exp _GTE exp        { bytecode_addcode(b, GTE);  }
   | exp '>' exp         { bytecode_addcode(b, GT);   }
   | exp '&' exp         { bytecode_addcode(b, AND);  }
   | exp '^' exp         { bytecode_addcode(b, XOR);  }
   | exp '|' exp         { bytecode_addcode(b, OR);   }
   | exp _SHL exp        { bytecode_addcode(b, SHL);  }
   | exp _SHR exp        { bytecode_addcode(b, SHR);  }
   | exp '+' exp         { bytecode_addcode(b, ADD);  }
   | exp '-' exp         { bytecode_addcode(b, SUB);  }
   | exp '*' exp         { bytecode_addcode(b, MUL);  }
   | exp '/' exp         { bytecode_addcode(b, DIV);  }
   | exp _IDIV exp       { bytecode_addcode(b, IDIV); }
   | exp '%' exp         { bytecode_addcode(b, MOD);  }
   | exp _POW exp        { bytecode_addcode(b, POW);  }
   | '#' exp %prec _LEN  { bytecode_addcode(b, LEN);  }
   | exp CONCAT exp      { bytecode_addcode(b, CAT);  }
   | '!' exp %prec _BNOT { bytecode_addcode(b, BNOT);  }
   | '~' exp %prec _NOT  { bytecode_addcode(b, NOT);  }
   | '-' exp %prec _NEG  { bytecode_addcode(b, NEG);  }
   | exp '[' exp ']'     { bytecode_addcode(b, LOOKUP);  }
   | exp '[' lookup_pos ']'
   | '?' exp %prec ISNIL { bytecode_addcode(b, PUSH_Nil); bytecode_addcode(b, EQ); }
   | '(' exp ')'
   ;

// strings
string: STRING { 
            bytecode_addcode(b, PUSH_S); 
            bytecode_addcodestr(b, $1.str); 
            free($1.str); 
        }
   ;

strings: string
       | string strings { bytecode_addcode(b, CAT); }
       ;

// arrays
array: '[' { bytecode_addcode(b, PUSHARY); } array_items ']'
     ;

array_items: %empty
           | array_item
           | array_item ',' array_items
           ;

array_item: exp { bytecode_addcode(b, APPEND); }
          ;

// table initialization
table: '{' { bytecode_addcode(b, PUSHTBL); } tbl_items '}'
     ;

tbl_items: %empty
         | tbl_item
         | tbl_item ',' tbl_items
         ;

tbl_item: IDENTIFIER { 
              bytecode_addcode(b, PUSH_S);
              bytecode_addcodestr(b, $1);
           } ':' exp { bytecode_addcode(b, TBLSET); }
        | '[' exp ']' ':' exp { bytecode_addcode(b, TBLSET); }
        ;

// lookup position
lookup_pos: exp ':' exp  { bytecode_addcode(b, SLICE); }
          | ':'          { bytecode_addcode(b, PUSH_N); bytecode_addcodef64(b, 0); } 
            exp          { bytecode_addcode(b, SLICE); }
          | exp ':'      { bytecode_addcode(b, PUSH_Nil); 
                           bytecode_addcode(b, SLICE); }
          | ':'          { bytecode_addcode(b, PUSH_N); bytecode_addcodef64(b, 0); 
                           bytecode_addcode(b, PUSH_Nil);
                           bytecode_addcode(b, SLICE); }
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
             $<label>2 = bytecode_createlabel(b);
             bytecode_addcode(b, Bfalse); bytecode_addcodelabel(b, $<label>2);
         } exp {
             bytecode_addcode(b, Bfalse); bytecode_addcodelabel(b, $<label>2);
             bytecode_addcode(b, PUSH_Bt);

             Label is_true = bytecode_createlabel(b);
             bytecode_addcode(b, JMP); bytecode_addcodelabel(b, is_true);

             bytecode_setlabel(b, $<label>2); 
             bytecode_addcode(b, PUSH_Bf);

             bytecode_setlabel(b, is_true); 
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
             $<label>2 = bytecode_createlabel(b);
             bytecode_addcode(b, Btrue); bytecode_addcodelabel(b, $<label>2);
         } exp {
             bytecode_addcode(b, Btrue); bytecode_addcodelabel(b, $<label>2);
             bytecode_addcode(b, PUSH_Bf);

             Label is_false = bytecode_createlabel(b);
             bytecode_addcode(b, JMP); bytecode_addcodelabel(b, is_false);

             bytecode_setlabel(b, $<label>2); 
             bytecode_addcode(b, PUSH_Bt);

             bytecode_setlabel(b, is_false); 
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
             $<label>2 = bytecode_createlabel(b);
             bytecode_addcode(b, Bfalse); bytecode_addcodelabel(b, $<label>2);
         } exp ':' {
             $<label>4 = bytecode_createlabel(b);
             bytecode_addcode(b, JMP); bytecode_addcodelabel(b, $<label>4);
             
             bytecode_setlabel(b, $<label>2); 
         } exp {
             bytecode_setlabel(b, $<label>4); 
         }
       ;

%%


int parse(Bytecode* b, const char* code)
{
    // parse code
    void *scanner;
    yylex_init_extra(b, &scanner);
    yy_scan_bytes(code, strlen(code), scanner);
    int r = yyparse(scanner, b);
    yylex_destroy(scanner);
    return r;
}


void yyerror(void* scanner, Bytecode* bc, const char *s)
{
	fprintf(stderr, "%s\n", s);
}

// vim: ts=4:sw=4:sts=4:expandtab
