#ifndef ZOE_LIB_GLOBAL_H_
#define ZOE_LIB_GLOBAL_H_

#include <stdio.h>

// definitions
#define STACK_MAX 20

// when a function/method is not implemented, causes a error and abort
#define TODO(def) \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"") \
    inline def { fprintf(stderr, "not implemented: \033[1;31m" #def "\033[0m\n"); abort(); } \
    _Pragma("GCC diagnostic pop")

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
