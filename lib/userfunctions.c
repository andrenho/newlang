#include "lib/userfunctions.h"

#include <stdio.h>
#include <stdlib.h>

static void default_error(const char* str)
{
    fprintf(stderr, "zoe: %s\n", str);
    abort();
}

UserFunctions default_userfunctions = {
    .realloc = realloc,
    .free    = free,
    .error   = default_error,
};

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
