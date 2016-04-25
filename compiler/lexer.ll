%{

// ignore warnings from G++
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"

// set start conditions from within bison
#define YY_HEADER_EXPORT_START_CONDITIONS

#include <cstdlib>
#include <cstring>

#include "compiler/parser.hh"

%}

/* generate a header file */
/* %option header-file="compiler/lexer.h" */

/* do not call `yywrap` upon end-of-file */
%option noyywrap

/* make the lexer reentrant */
%option reentrant

/* we are using a bison-compatible scanner */
%option bison-bridge

%%


%%


// vim: ts=4:sw=4:sts=4:expandtab:syntax=lex
