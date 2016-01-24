#include "lib/zoe.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lib/bytecode.h"
#include "lib/opcode.h"
#include "lib/zworld.h"

typedef struct Zoe {
    ZWorld*  world;
    ERROR    errorf;
    ZValue*  stack[STACK_MAX];
    STPOS    stack_sz;
#ifdef DEBUG
    bool     debug_asm;
#endif
} Zoe;

// {{{ CONSTRUCTOR/DESTRUCTOR

static void default_error(const char* msg)
{
    fprintf(stderr, "zoe error: %s\n", msg);
    abort();
}


Zoe*
zoe_createvm(ERROR errorf)
{
    if(!errorf) {
        errorf = default_error;
    }
    Zoe* Z = calloc(sizeof(Zoe), 1);
    Z->world = zworld_new(errorf);
    Z->errorf = errorf;
#ifdef DEBUG
    Z->debug_asm = false;
#endif
    return Z;
}


void
zoe_free(Zoe* Z)
{
    zoe_pop(Z, zoe_stacksize(Z));
    zworld_free(Z->world);
    free(Z);
}

// }}}

// {{{ PRIVATE, LOW LEVEL STACK ACCESS

// the following functions are not static so they can be tested
// in tests/tests.c

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-prototypes"

ZValue* zoe_stack_pushexisting(Zoe* Z, ZValue* existing)
{
    if(Z->stack_sz == STACK_MAX-1) {
        zoe_error(Z, "Stack overflow.");
        return NULL;
    }

    Z->stack[Z->stack_sz] = existing;
    ++Z->stack_sz;

    zworld_inc(Z->world, existing);

    return existing;
}


ZValue* zoe_stack_pushnew(Zoe* Z, ZType type)
{
    ZValue* value = zworld_alloc(Z->world, type);
    return zoe_stack_pushexisting(Z, value);
}


void zoe_stack_remove(Zoe* Z, STPOS pos)
{
    pos = zoe_absindex(Z, pos);
    if(pos >= Z->stack_sz) {
        zoe_error(Z, "Position > stack size.");
        return NULL;
    }
    
    zworld_dec(Z->world, Z->stack[pos]);
    if(Z->stack_sz > 1) {
        memmove(&Z->stack[pos], &Z->stack[pos+1], sizeof(ZValue*) * (STACK_MAX - pos));
    }
    --Z->stack_sz;
}


void zoe_stack_pop(Zoe* Z)
{
    if(Z->stack_sz == 0) {
        zoe_error(Z, "Stack overflow.");
        return;
    }
    
    zoe_stack_remove(Z, -1);
}


ZValue* zoe_stack_get(Zoe* Z, STPOS pos)
{
    pos = zoe_absindex(Z, pos);
    if(pos >= Z->stack_sz) {
        zoe_error(Z, "Position > stack size.");
        return NULL;
    }
    return Z->stack[pos];
}

#pragma GCC diagnostic pop

// }}}

// {{{ HIGH LEVEL STACK ACCESS

STPOS
zoe_stacksize(Zoe* Z)
{
    return Z->stack_sz;
}


STPOS
zoe_absindex(Zoe* Z, STPOS pos)
{
    STPOS i = (pos >= 0) ? pos : zoe_stacksize(Z) + pos;
    assert(i >= 0);
    return i;
}

void
zoe_pushnil(Zoe* Z)
{
    zoe_stack_pushnew(Z, NIL);
}

void
zoe_pushboolean(Zoe* Z, bool b)
{
    ZValue* value = zoe_stack_pushnew(Z, BOOLEAN);
    value->boolean = b;
}

void
zoe_pushnumber(Zoe* Z, double n)
{
    ZValue* value = zoe_stack_pushnew(Z, NUMBER);
    value->number = n;
}

void
zoe_pushbfunction(Zoe* Z, uint8_t* data, size_t sz)
{
    ZValue* value = zoe_stack_pushnew(Z, FUNCTION);
    value->function.type = BYTECODE;
    value->function.bytecode.data = malloc(sz);
    memcpy(value->function.bytecode.data, data, sz);
    value->function.bytecode.sz = sz;
}

void
zoe_pushstring(Zoe* Z, char* s)
{
    ZValue* value = zoe_stack_pushnew(Z, STRING);
    value->string = strdup(s);
}

void zoe_pusharray(Zoe* Z)
{
    ZValue* value = zoe_stack_pushnew(Z, ARRAY);
    value->array.n = 0;
    value->array.items = NULL;
}

void
zoe_pop(Zoe* Z, size_t count)
{
    for(size_t i=0; i<count; ++i) {
        zoe_stack_pop(Z);
    }
}

ZType
zoe_gettype(Zoe* Z, STPOS n)
{
    n = zoe_absindex(Z, n);
    assert(n >= 0 && n < zoe_stacksize(Z));
    return zoe_stack_get(Z, n)->type;
}

static ZValue* zoe_checktype(Zoe* Z, STPOS n, ZType type) 
{
    ZValue* value = zoe_stack_get(Z, n);
    if(value->type != type) {
        zoe_error(Z, "Expected type '%s', found '%s'.", zvalue_typename(type), zvalue_typename(value->type));
    }
    return value;
}

void
zoe_getnil(Zoe* Z, STPOS i)
{
    zoe_checktype(Z, i, NIL);
}

bool
zoe_getboolean(Zoe* Z, STPOS i)
{
    return zoe_checktype(Z, i, BOOLEAN)->boolean;
}

double
zoe_getnumber(Zoe* Z, STPOS i)
{
    return zoe_checktype(Z, i, NUMBER)->number;
}

char const*
zoe_getstring(Zoe* Z, STPOS i)
{
    return zoe_checktype(Z, i, STRING)->string;
}

void
zoe_popnil(Zoe* Z)
{
    zoe_checktype(Z, -1, NIL);
    zoe_stack_pop(Z);
}

bool
zoe_popboolean(Zoe* Z)
{
    bool r = zoe_checktype(Z, -1, BOOLEAN)->boolean;
    zoe_stack_pop(Z);
    return r;
}

double
zoe_popnumber(Zoe* Z)
{
    double r = zoe_checktype(Z, -1, NUMBER)->number;
    zoe_stack_pop(Z);
    return r;
}

char*
zoe_popstring(Zoe* Z)
{
    char* r = strdup(zoe_checktype(Z, -1, STRING)->string);
    zoe_stack_pop(Z);
    return r;
}

inline ZType       zoe_peektype(Zoe* Z)    { return zoe_gettype(Z, -1); }
inline void        zoe_peeknil(Zoe* Z)     { zoe_getnil(Z, -1); }
inline bool        zoe_peekboolean(Zoe* Z) { return zoe_getboolean(Z, -1); }
inline double      zoe_peeknumber(Zoe* Z)  { return zoe_getnumber(Z, -1); }
inline char const* zoe_peekstring(Zoe* Z)  { return zoe_getstring(Z, -1); }

// }}}

// {{{ ARRAY MANAGEMENT

void zoe_arrayappend(Zoe* Z)
{
    // get array
    ZValue* varray = zoe_stack_get(Z, -2);
    if(varray->type != ARRAY) {
        zoe_error(Z, "Only arrays can be appended.");
        return;
    }
    ZArray* array = &varray->array;

    // add item to array
    ZValue* value = zoe_stack_get(Z, -1);
    ++array->n;
    array->items = realloc(array->items, array->n * sizeof(ZValue*));
    array->items[array->n-1] = value;
    zworld_inc(Z->world, value);

    // pop item from stack
    zoe_pop(Z, 1);
}


static void zoe_arraymul(Zoe* Z)
{
    uint64_t mul = zoe_popnumber(Z);
    if(mul < 1) {
        zoe_error(Z, "Arrays can only be multiplied by values >= 1");
        return;
    }

    ZArray* array = &zoe_stack_get(Z, -1)->array;

    // create new array
    ZValue* narr = zoe_stack_pushnew(Z, ARRAY);
    narr->array.n = array->n * mul;
    narr->array.items = malloc(narr->array.n * sizeof(ZValue*));
    for(size_t i=0; i<mul; ++i) {
        for(size_t j=0; j<array->n; ++j) {
            ZValue* value = array->items[j];
            narr->array.items[(i*array->n) + j] = value;
            zworld_inc(Z->world, value);
        }
    }

    // remove old array
    zoe_stack_remove(Z, -2);
}


void zoe_arrayslice(Zoe* Z)
{
    // read data
    int64_t start, end;
    ZArray* oldary = &zoe_checktype(Z, -3, ARRAY)->array;
    if(zoe_gettype(Z, -1) == NIL) {
        end = oldary->n;
        zoe_pop(Z, 1);
    } else {
        end = zoe_popnumber(Z);
    }
    start = zoe_popnumber(Z);
    int64_t old_n = oldary->n;
    start = (start >=0) ? start : (old_n + start);
    end = (end >=0) ? end : (old_n + end);

    if(start > old_n || end > old_n) {
        zoe_error(Z, "Subscript out of range.");
        return;
    }
    if(start >= end) {
        zoe_error(Z, "Inverted slicing (start <= end)");
    }

    // add to new array
    ZArray* newary = &zoe_stack_pushnew(Z, ARRAY)->array;
    newary->n = (end-start);
    newary->items = malloc(newary->n * sizeof(ZValue*));
    for(int i=0, j=start; j<end; ++i, ++j) {
        ZValue* value = oldary->items[j];
        newary->items[i] = value;
        zworld_inc(Z->world, value);
    }

    // remove old array from stack
    zoe_stack_remove(Z, -2);
}


// }}}

// {{{ ERROR MANAGEMENT

void zoe_error(Zoe* Z, char* fmt, ...)
{
    static char buf[1024];
    va_list a;

    va_start(a, fmt);
    vsnprintf(buf, sizeof buf, fmt, a);
    va_end(a);

    Z->errorf(buf);
}

// }}}

// {{{ CODE EXECUTION

// {{{ OPERATIONS

void zoe_len(Zoe* Z)
{
    ZType t = zoe_gettype(Z, -1);
    if(t == STRING) {
        char* str = zoe_popstring(Z);
        zoe_pushnumber(Z, strlen(str));
        free(str);
    } else if(t == ARRAY) {
        ZArray *array = &zoe_stack_get(Z, -1)->array;
        int n = array->n;
        zoe_pop(Z, 1);
        zoe_pushnumber(Z, n);
    } else {
        zoe_error(Z, "Expected string or array, found %s\n", zvalue_typename(t));
    }
}


void zoe_lookup(Zoe* Z)
{
    ZType t = zoe_gettype(Z, -2);
    if(t == STRING) {
        int64_t i = zoe_popnumber(Z);
        char* str = zoe_popstring(Z);
        uint64_t k = (i >= 0) ? (uint64_t)i : strlen(str) + i;
        // TODO - raise error if number is higher than the number of characters in the string
        zoe_pushstring(Z, (char[]) { str[k], 0 });
        free(str);
    } else if(t == ARRAY) {
        int64_t i = zoe_popnumber(Z);
        ZValue* value = zoe_stack_get(Z, -1);
        uint64_t k = (i >= 0) ? (uint64_t)i : value->array.n + i;
        if(k >= value->array.n) {
            zoe_error(Z, "Subscript out of index.");
            return;
        }
        zoe_stack_pushexisting(Z, value->array.items[k]);
        zoe_stack_remove(Z, -2);  // remove array
    } else {
        zoe_error(Z, "Expected string or array, found %s\n", zvalue_typename(t));
    }
}


static bool zoe_eq(Zoe* Z, ZValue* a, ZValue* b)
{
    if(a->type != b->type) {
        return false;
    } else {
        switch(a->type) {
            case NIL:
                return true;
            case BOOLEAN:
                return a->boolean == b->boolean;
            case NUMBER:
                return fabs(a->number - b->number) < DBL_EPSILON;
            case STRING:
                return strcmp(a->string, b->string) == 0;
            case ARRAY:
                if(a->array.n != b->array.n) {
                    return false;
                }
                for(size_t i=0; i<a->array.n; ++i) {
                    if(!zoe_eq(Z, a->array.items[i], b->array.items[i])) {
                        return false;
                    }
                }
                return true;
            case FUNCTION:
                zoe_error(Z, "function comparison not implemented yet"); // TODO
                abort();
            default:
                zoe_error(Z, "equality does not exists for type %s", zvalue_typename(a->type));
                return false;
        }
    }
}


void zoe_concat(Zoe* Z)
{
    ZType t = zoe_gettype(Z, -2);
    if(t == STRING) {
        char *b = zoe_popstring(Z),
             *a = zoe_popstring(Z);
        a = realloc(a, strlen(a) + strlen(b) + 1);
        strcat(a, b);
        zoe_pushstring(Z, a);
        free(b);
        free(a);
    } else if(t == ARRAY) {
        // get arrays
        ZValue *a1 = zoe_stack_get(Z, -2),
               *a2 = zoe_checktype(Z, -1, ARRAY);

        // create new array
        ZValue *anew = zoe_stack_pushnew(Z, ARRAY);
        anew->array.n = a1->array.n + a2->array.n;
        anew->array.items = malloc(anew->array.n * sizeof(ZValue*));
        
        // add data
        size_t i=0;
        ZValue *arr[] = { a1, a2 };
        for(size_t j=0; j<2; ++j) {
            for(size_t k=0; k < arr[j]->array.n; ++k) {
                ZValue* value = arr[j]->array.items[k];
                anew->array.items[i++] = value;
                zworld_inc(Z->world, value);
            }
        }

        // pop old arrays
        zoe_stack_remove(Z, -2);
        zoe_stack_remove(Z, -2);
    } else {
        zoe_error(Z, "Expected string or array, found %s\n", zvalue_typename(t));
    }
}


void zoe_oper(Zoe* Z, Operator oper)
{
    if(oper == ZOE_NEG) {
        zoe_pushnumber(Z, -zoe_popnumber(Z));
    } else if(oper == ZOE_NOT) {
        double d = zoe_popnumber(Z);
        int64_t c = (int64_t)d;
        zoe_pushnumber(Z, ~c);
    } else if(oper == ZOE_BNOT) {
        zoe_pushboolean(Z, !zoe_popboolean(Z));
    } else if(oper == ZOE_EQ) {
        ZValue *b = zoe_stack_get(Z, -1),
               *a = zoe_stack_get(Z, -2);
        bool r = zoe_eq(Z, a, b);
        zoe_pop(Z, 2);
        zoe_pushboolean(Z, r);
    } else if(oper == ZOE_MUL && zoe_gettype(Z, -2) == ARRAY) {
        zoe_arraymul(Z);
    } else {
        double b = zoe_popnumber(Z), 
               a = zoe_popnumber(Z);
        double c = 0.0;

        switch(oper) {
            case ZOE_ADD:  c = a + b; break;
            case ZOE_SUB:  c = a - b; break;
            case ZOE_MUL:  c = a * b; break;
            case ZOE_DIV:  c = a / b; break;
            case ZOE_IDIV: c = floor(a / b); break;
            case ZOE_MOD:  c = fmod(a, b); break;
            case ZOE_POW:  c = pow(a, b); break;
            case ZOE_AND:  c = (int64_t)a & (int64_t)b; break;
            case ZOE_XOR:  c = (int64_t)a ^ (int64_t)b; break;
            case ZOE_OR:   c = (int64_t)a | (int64_t)b; break;
            case ZOE_SHL:  c = (int64_t)a << (int64_t)b; break;
            case ZOE_SHR:  c = (int64_t)a >> (int64_t)b; break;
            
            case ZOE_LT:  zoe_pushboolean(Z, a < b);  return;
            case ZOE_LTE: zoe_pushboolean(Z, a <= b); return;
            case ZOE_GT:  zoe_pushboolean(Z, a > b);  return;
            case ZOE_GTE: zoe_pushboolean(Z, a >= b); return;

            case ZOE_NEG:  // pleases gcc
            case ZOE_EQ:
            case ZOE_NOT:
            case ZOE_BNOT:
            default:
                zoe_error(Z, "Invalid operator (code %d)", oper);
                return;
        }
        zoe_pushnumber(Z, c);
    }
}

// }}}

// {{{ EVAL & EXECUTION

void zoe_eval(Zoe* Z, const char* code)
{
    Bytecode* bc = bytecode_newfromcode(code, Z->errorf);

    uint8_t* buffer;
    size_t sz = bytecode_generatezb(bc, &buffer);

    // we do all this so we won't have to copy the buffer
    ZValue* value = zoe_stack_pushnew(Z, FUNCTION);
    value->function.bytecode.data = buffer;
    value->function.bytecode.sz = sz;

    bytecode_free(bc);
}


#ifdef DEBUG
static void zoe_dbgopcode(uint8_t* code, uint64_t p);
static void zoe_dbgstack(Zoe* Z);
#endif

static void zoe_execute(Zoe* Z, uint8_t* data, size_t sz)
{
    Bytecode* bc = bytecode_newfromzb(data, sz, Z->errorf);
    uint64_t p = 0;

    while(p < bc->code_sz) {
#ifdef DEBUG
        if(Z->debug_asm) {
            zoe_dbgopcode(bc->code, p);
        }
#endif
        Opcode op = bc->code[p];
        switch(op) {
            // 
            // stack
            //
            case PUSH_Nil: zoe_pushnil(Z); ++p; break;
            case PUSH_Bt: zoe_pushboolean(Z, true); ++p; break;
            case PUSH_Bf: zoe_pushboolean(Z, false); ++p; break;
            case PUSH_N: {
                    double n;
                    memcpy(&n, &bc->code[p+1], 8);
                    zoe_pushnumber(Z, n);
                    p += 9;
                }
                break;
            case PUSH_S: 
                zoe_pushstring(Z, (char*)&bc->code[p+1]); 
                p += 2 + strlen((char*)&bc->code[p+1]);
                break;
            case POP: zoe_pop(Z, 1); ++p; break;

            //
            // oper
            //
            case ADD:  zoe_oper(Z, ZOE_ADD);  ++p; break;
            case SUB:  zoe_oper(Z, ZOE_SUB);  ++p; break;
            case MUL:  zoe_oper(Z, ZOE_MUL);  ++p; break;
            case DIV:  zoe_oper(Z, ZOE_DIV);  ++p; break;
            case IDIV: zoe_oper(Z, ZOE_IDIV); ++p; break;
            case MOD:  zoe_oper(Z, ZOE_MOD);  ++p; break;
            case POW:  zoe_oper(Z, ZOE_POW);  ++p; break;
            case NEG:  zoe_oper(Z, ZOE_NEG);  ++p; break;
            case AND:  zoe_oper(Z, ZOE_AND);  ++p; break;
            case OR:   zoe_oper(Z, ZOE_OR);   ++p; break;
            case XOR:  zoe_oper(Z, ZOE_XOR);  ++p; break;
            case SHL:  zoe_oper(Z, ZOE_SHL);  ++p; break;
            case SHR:  zoe_oper(Z, ZOE_SHR);  ++p; break;
            case NOT:  zoe_oper(Z, ZOE_NOT);  ++p; break;
            case BNOT: zoe_oper(Z, ZOE_BNOT); ++p; break;
            case LT:   zoe_oper(Z, ZOE_LT);   ++p; break;
            case LTE:  zoe_oper(Z, ZOE_LTE);  ++p; break;
            case GT:   zoe_oper(Z, ZOE_GT);   ++p; break;
            case GTE:  zoe_oper(Z, ZOE_GTE);  ++p; break;
            case EQ:   zoe_oper(Z, ZOE_EQ);   ++p; break;
            case CAT:  zoe_concat(Z);         ++p; break;
            case LEN:  zoe_len(Z);            ++p; break;
            case LOOKUP: zoe_lookup(Z);       ++p; break;

            //
            // branches
            //
            case JMP: memcpy(&p, &bc->code[p+1], 8); break;
            case Bfalse: {
                    uint8_t n;
                    memcpy(&n, &bc->code[p+1], 8);
                    if(!zoe_popboolean(Z)) {
                        p = n;
                    } else {
                        p += 9;
                    }
                }
                break;
            case Btrue: {
                    uint8_t n;
                    memcpy(&n, &bc->code[p+1], 8);
                    if(zoe_popboolean(Z)) {
                        p = n;
                    } else {
                        p += 9;
                    }
                }
                break;

            //
            // array
            //
            case PUSHARY: zoe_pusharray(Z);   ++p; break;
            case APPEND:  zoe_arrayappend(Z); ++p; break;
            case SLICE:   zoe_arrayslice(Z);  ++p; break;
            
            //
            // others
            //
            case END: p = bc->code_sz; break;
            default:
                zoe_error(Z, "Invalid opcode 0x%02X.", op);
        }
#ifdef DEBUG
        if(Z->debug_asm) {
            zoe_dbgstack(Z);
        }
#endif
    }

    bytecode_free(bc);
}


void zoe_call(Zoe* Z, int n_args)
{
    (void) n_args;

    STPOS initial = zoe_stacksize(Z);

    // load function
    ZValue* value = zoe_stack_get(Z, -1);
    if(value->type != FUNCTION) {
        zoe_error(Z, "Expected function, found '%s'.\n", zvalue_typename(value->type));
    }

    ZFunction* f = &value->function;
    if(f->type != BYTECODE) {
        zoe_error(Z, "Can only execute code in ZB format.");
    }

    // copy function contents
    // TODO - this is slow! we should just move the pointer, and remove the 
    // function from the stack without freeing the pointer
    size_t sz = f->bytecode.sz;
    uint8_t* data = malloc(sz);
    memcpy(data, f->bytecode.data, sz);

    // remove function from stack
    zoe_stack_pop(Z);

    // execute
    zoe_execute(Z, data, sz);

    // verify if the stack has now is the same size as the beginning
    // (-1 function +1 return argument)
    if(zoe_stacksize(Z) != initial) {
        zoe_error(Z, "Function should have returned exaclty one argument.");
    }

    free(data);
}

// }}}

// }}}

// {{{ DEBUGGING

// {{{ STRING ESCAPING

static char* zoe_escapestring(const char* s)
{
    char* buf = calloc((strlen(s) * 2) + 3, 1);
    int a = 0, b = 0;
    buf[b++] = '\'';
    while(s[a]) {
        if(s[a] == 10) {
            buf[b++] = '\\'; buf[b++] = 'n';
            ++a;
        } else if(s[a] == 13) {
            buf[b++] = '\\'; buf[b++] = 'r';
            ++a;
        } else if(s[a] == '\\' || s[a] == '\'') {
            buf[b++] = '\\';
            buf[b++] = s[a++];
        } else if(s[a] >= 32 && s[a] < 127) {
            buf[b++] = s[a++];
        } else {
            snprintf(&buf[b], 5, "\\x%02X", (uint8_t)s[a]);
            b += 4;
            ++a;
        }
    }
    buf[b++] = '\'';
    buf[b++] = 0;
    return buf;
}

// }}}

#ifdef DEBUG

// {{{ FORMATTING

static int aprintf(char** ptr, const char* fmt, ...) __attribute__ ((format (printf, 2, 3)));
static int aprintf(char** ptr, const char* fmt, ...)
{
    va_list ap;

    size_t cur_sz = (*ptr ? strlen(*ptr) : 0);

    va_start(ap, fmt);
    int new_sz = cur_sz + vsnprintf(NULL, 0, fmt, ap) + 1;
    va_end(ap);
    *ptr = realloc(*ptr, new_sz);
    va_start(ap, fmt);
    int r = vsnprintf((*ptr) + cur_sz, new_sz, fmt, ap);
    va_end(ap);

    return r;
}


static int sprint_dbl(char* buf, size_t nbuf, uint8_t* array, size_t pos)
{
    double d;
    memcpy(&d, &array[pos], __SIZEOF_DOUBLE__);
    int r = snprintf(buf, nbuf, "%0.14f", d);
    int lg;
    while(lg = strlen(buf)-1, (buf[lg] == '0' || buf[lg] == '.') && lg > 0) {
        buf[lg] = '\0';
        --r;
    }
    return r;
}


static int sprint_uint64(char* buf, size_t nbuf, uint8_t* array, size_t pos)
{
    uint64_t n;
    memcpy(&n, &array[pos], 8);
    return snprintf(buf, nbuf, "0x%" PRIx64, n);
}

// }}}

// {{{ OPCODE DISASSEMBLY

static int sprint_code(char* buf, size_t nbuf, uint8_t* code, uint64_t p) {
    Opcode op = (Opcode)code[p];
    switch(op) {
        case PUSH_Nil: snprintf(buf, nbuf, "PUSH_Nil"); return 1;
        case PUSH_Bt:  snprintf(buf, nbuf, "PUSH_Bt"); return 1;
        case PUSH_Bf:  snprintf(buf, nbuf, "PUSH_Bf"); return 1;
        case PUSH_N: {
                char xbuf[128]; sprint_dbl(xbuf, sizeof xbuf, code, p+1);
                snprintf(buf, nbuf, "PUSH_N  %s", xbuf);
                return 9;
            }
        case PUSH_S: {
                char* sbuf = zoe_escapestring((char*)&code[p+1]);
                snprintf(buf, nbuf, "PUSH_S  %s", sbuf);
                free(sbuf);
                return 2 + strlen((char*)&code[p+1]);
            }
        case POP:  snprintf(buf, nbuf, "POP");  return 1;
        case ADD:  snprintf(buf, nbuf, "ADD");  return 1;
        case SUB:  snprintf(buf, nbuf, "SUB");  return 1;
        case MUL:  snprintf(buf, nbuf, "MUL");  return 1;
        case DIV:  snprintf(buf, nbuf, "DIV");  return 1;
        case IDIV: snprintf(buf, nbuf, "IDIV"); return 1;
        case MOD:  snprintf(buf, nbuf, "MOD");  return 1;
        case POW:  snprintf(buf, nbuf, "POW");  return 1;
        case NEG:  snprintf(buf, nbuf, "NEG");  return 1;
        case AND:  snprintf(buf, nbuf, "AND");  return 1;
        case XOR:  snprintf(buf, nbuf, "XOR");  return 1;
        case OR:   snprintf(buf, nbuf, "OR") ;  return 1;
        case SHL:  snprintf(buf, nbuf, "SHL");  return 1;
        case SHR:  snprintf(buf, nbuf, "SHR");  return 1;
        case NOT:  snprintf(buf, nbuf, "NOT");  return 1;
        case BNOT: snprintf(buf, nbuf, "BNOT"); return 1;
        case LT:   snprintf(buf, nbuf, "LT");   return 1;
        case LTE:  snprintf(buf, nbuf, "LTE");  return 1;
        case GT:   snprintf(buf, nbuf, "GT");   return 1;
        case GTE:  snprintf(buf, nbuf, "GTE");  return 1;
        case EQ:   snprintf(buf, nbuf, "EQ");   return 1;
        case CAT:  snprintf(buf, nbuf, "CAT");  return 1;
        case LEN:  snprintf(buf, nbuf, "LEN");  return 1;
        case LOOKUP: snprintf(buf, nbuf, "LOOKUP");  return 1;
        case JMP: {
                char xbuf[128]; sprint_uint64(xbuf, sizeof xbuf, code, p+1);
                snprintf(buf, nbuf, "JMP     %s", xbuf);
                return 9;
            }
        case Bfalse: {
                char xbuf[128]; sprint_uint64(xbuf, sizeof xbuf, code, p+1);
                snprintf(buf, nbuf, "Bfalse  %s", xbuf);
                return 9;
            }
        case Btrue: {
                char xbuf[128]; sprint_uint64(xbuf, sizeof xbuf, code, p+1);
                snprintf(buf, nbuf, "Btrue   %s", xbuf);
                return 9;
            }
        case PUSHARY: snprintf(buf, nbuf, "PUSHARY"); return 1;
        case APPEND:  snprintf(buf, nbuf, "APPEND");  return 1;
        case SLICE:   snprintf(buf, nbuf, "SLICE");   return 1;
        case END:     snprintf(buf, nbuf, "END");     return 1;
        default:
            snprintf(buf, nbuf, "Invalid opcode %02X\n", (uint8_t)op);
    }
    return 1;
}


void zoe_disassemble(Zoe* Z)
{
    char* buf = NULL;

    // load function
    ZValue* value = zoe_stack_get(Z, -1);
    if(value->type != FUNCTION) {
        zoe_error(Z, "Expected function, found '%s'.\n", zvalue_typename(value->type));
    }

    ZFunction* f = &value->function;
    if(f->type != BYTECODE) {
        zoe_error(Z, "Can only execute code in ZB format.");
    }

    Bytecode* bc = bytecode_newfromzb(f->bytecode.data, f->bytecode.sz, Z->errorf);
    uint64_t p = 0;

    while(p < bc->code_sz) {
        // address
        int ns = aprintf(&buf, "%08" PRIx64 ":\t", p);
        // code
        static char cbuf[128];
        int n = sprint_code(cbuf, sizeof cbuf, bc->code, p);
        ns += aprintf(&buf, "%s", cbuf);
        // spaces
        aprintf(&buf, "%*s", 32-ns, " ");
        // hex code
        for(uint8_t i=0; i<n; ++i) {
            aprintf(&buf, "%02X ", bc->code[p+i]);
        }
        aprintf(&buf, "\n");

        p += n;
    }

    bytecode_free(bc);

    if(buf) {
        zoe_pushstring(Z, buf);
    } else {
        zoe_pushstring(Z, "");
    }
    free(buf);
}

// }}}

// {{{ ASSEMBLY DEBUGGER

void zoe_asmdebugger(Zoe* Z, bool value)
{
    Z->debug_asm = value;
}


void zoe_gcdebugger(Zoe* Z, bool value)
{
    zworld_gcdebugger(Z->world, value);
}


static void zoe_dbgopcode(uint8_t* code, uint64_t p)
{
    int ns = printf("%08" PRIx64 ":\t", p);

    char buf[128];
    sprint_code(buf, sizeof buf, code, p);
    ns += printf("%s", buf);
    printf("%*s", 32-ns, " ");
}


static void zoe_dbgstack(Zoe* Z)
{
    printf("< ");
    for(int i=zoe_stacksize(Z)-1; i >= 0; --i) {
        if(i != (zoe_stacksize(Z)-1)) {
            printf(", ");
        }
        zoe_inspect(Z, 0-1-i);
        char* buf = zoe_popstring(Z);
        printf("%s", buf);
        free(buf);
    }
    printf(" >\n");
}

// }}}

#endif

// {{{ INSPECTION


static char* zvalue_inspect(Zoe* Z, ZValue* value)
{
    switch(value->type) {
        case NIL:
            return strdup("nil");
        case BOOLEAN: {
                return strdup(value->boolean ? "true" : "false");
            }
        case NUMBER: {
                char* buf = calloc(128, 1);
                snprintf(buf, 127, "%0.14f", value->number);
                // remove zeroes at the and
                int lg;
                while(lg = strlen(buf)-1, (buf[lg] == '0' || buf[lg] == '.') && lg > 0) {
                    bool br = (buf[lg] == '.');
                    buf[lg] = '\0';
                    if(br) 
                        break;
                }
                return buf;
            }
        case FUNCTION:
            return strdup("function");
        case STRING: {
                char* buf = zoe_escapestring(value->string);
                return buf;
            }
        case ARRAY: {
                char* buf = strdup("[");
                for(size_t i=0; i<value->array.n; ++i) {
                    bool last = (i == (value->array.n-1));
                    char* nbuf = zvalue_inspect(Z, value->array.items[i]);
                    buf = realloc(buf, strlen(buf) + strlen(nbuf) + (last ? 1 : 3));
                    strcat(buf, nbuf);
                    if(!last) {
                        strcat(buf, ", ");
                    }
                    free(nbuf);
                }
                buf = realloc(buf, strlen(buf)+2);
                strcat(buf, "]");
                return buf;
            }
        default: {
            zoe_error(Z, "Invalid type (code %d) in the stack.", value->type);
            return NULL;
        }
    }
}


void zoe_inspect(Zoe* Z, STPOS i)
{
    char *buf = zvalue_inspect(Z, zoe_stack_get(Z, i));
    zoe_pushstring(Z, buf);
    free(buf);
}

// }}}

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
