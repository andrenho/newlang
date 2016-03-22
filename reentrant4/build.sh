#!/bin/sh

bison -d parser.y
flex --header-file=lexer.h lexer.l

gcc -ggdb3 -c lex.yy.c
gcc -ggdb3 -c parser.tab.c

gcc -ggdb3 lex.yy.o parser.tab.o -o tmp -lm
