#ifndef LIB_GLOBAL_H_
#define LIB_GLOBAL_H_

#include <stdio.h>
#include <stdlib.h>

#define STACK_MAX 20

#define TODO { \
    _Pragma ("message \"Not implemented.\"") \
    fprintf(stderr, "`%s` not implemented.\n", __func__); \
    abort(); \
}

typedef void(*ERROR)(const char*);

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
