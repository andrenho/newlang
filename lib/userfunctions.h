#ifndef ZOE_USERFUNCTIONS_H_
#define ZOE_USERFUNCTIONS_H_

#include <stdlib.h>

typedef struct UserFunctions {
    void* (*realloc)(void*, size_t);
    void  (*free)(void*);
    void  (*error)(const char*);
} UserFunctions;

extern UserFunctions default_userfunctions;

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
