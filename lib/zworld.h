#ifndef LIB_ZWORLD_H_
#define LIB_ZWORLD_H_

#include "lib/global.h"
#include "lib/zvalue.h"

typedef struct ZWorld ZWorld;

// CONSTRUCTOR/DESTRUCTOR
ZWorld* zworld_new(ERROR errorf);
void    zworld_free(ZWorld* w);

// ALLOCATION/DEALLOCATION
ZValue* zworld_alloc(ZWorld* w, ZType type);
void    zworld_release(ZWorld* w, ZValue* value);

// MEMORY MANAGEMENT
void zworld_inc(ZWorld* w, ZValue* value);
void zworld_dec(ZWorld* w, ZValue* value);
void zworld_gc(ZWorld* w, ZValue* value);

// INFORMATION
size_t zworld_ref_count(ZWorld* w);

// DEBUGGING
#ifdef DEBUG
void zworld_gcdebugger(ZWorld* w, bool value);
#endif

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c