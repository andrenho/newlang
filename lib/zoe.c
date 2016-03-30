#include "lib/zoe.h"

#include <float.h>
#include <math.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "lib/bytecode.h"
#include "lib/opcode.h"

typedef struct Zoe {
    Stack*         stack;
    UserFunctions* uf;
} Zoe;

// {{{ CONSTRUCTOR/DESTRUCTOR

Zoe*
zoe_createvm(UserFunctions* uf)
{
    if(!uf) {
        uf = &default_userfunctions;
    }
    Zoe* Z = uf->realloc(NULL, sizeof(Zoe));
    memset(Z, 0, sizeof(Zoe));
    Z->stack = stack_new(uf);
    Z->uf = uf;
    return Z;
}


void
zoe_free(Zoe* Z)
{
    stack_free(Z->stack);
    Z->uf->free(Z);
}

// }}}

// {{{ HIGH LEVEL STACK ACCESS

STPOS
zoe_stacksize(Zoe* Z)
{
    return stack_size(Z->stack);
}


void
zoe_pushnil(Zoe* Z)
{
    stack_push(Z->stack, (ZValue){ .type=NIL });
}


void
zoe_pushboolean(Zoe* Z, bool b)
{
    stack_push(Z->stack, (ZValue){ .type=BOOLEAN, .boolean=b });
}


void
zoe_pushnumber(Zoe* Z, double n)
{
    stack_push(Z->stack, (ZValue){ .type=NUMBER, .number=n });
}


void
zoe_pushfunction(Zoe* Z, ZFunction f)
{
    stack_push(Z->stack, (ZValue){ .type=FUNCTION, .function=f });
}


void
zoe_pushstring(Zoe* Z, char* s)
{
    stack_push(Z->stack, (ZValue){ .type=STRING, .string=strdup(s) });
}


void
zoe_pop(Zoe* Z, int count)
{
    for(int i=0; i<count; ++i) {
        stack_popfree(Z->stack);
    }
}


ZType
zoe_peektype(Zoe* Z)
{
    return stack_peek(Z->stack, -1).type;
}


ZType
zoe_gettype(Zoe* Z, int n)
{
    return stack_peek(Z->stack, n).type;
}


static ZValue
zoe_checktype(Zoe* Z, ZType type_expected)
{
    ZValue value = stack_peek(Z->stack, -1);
    if(value.type != type_expected) {
        zoe_error(Z, "Expected %s, found %s\n", zoe_typename(type_expected), zoe_typename(value.type));
    }
    return value;
}


void
zoe_peeknil(Zoe* Z)
{
    zoe_checktype(Z, NIL);
}


bool
zoe_peekboolean(Zoe* Z)
{
    return zoe_checktype(Z, BOOLEAN).boolean;
}


double
zoe_peeknumber(Zoe* Z)
{
    return zoe_checktype(Z, NUMBER).number;
}


ZFunction
zoe_peekfunction(Zoe* Z)
{
    return zoe_checktype(Z, FUNCTION).function;
}


const char*
zoe_peekstring(Zoe* Z)
{
    return zoe_checktype(Z, STRING).string;
}


void
zoe_popnil(Zoe* Z)
{
    zoe_checktype(Z, NIL);
    stack_pop(Z->stack);
}


bool
zoe_popboolean(Zoe* Z)
{
    bool b = zoe_checktype(Z, BOOLEAN).boolean;
    stack_pop(Z->stack);
    return b;
}


double
zoe_popnumber(Zoe* Z)
{
    double n = zoe_checktype(Z, NUMBER).number;
    stack_pop(Z->stack);
    return n;
}


ZFunction
zoe_popfunction(Zoe* Z)
{
    ZFunction f = zoe_checktype(Z, FUNCTION).function;
    stack_pop(Z->stack);
    return f;
}


char*
zoe_popstring(Zoe* Z)
{
    char* s = zoe_checktype(Z, STRING).string;
    stack_pop(Z->stack);
    return s;
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

    Z->uf->error(buf);
}

// }}}

// {{{ INFORMATION

char* zoe_typename(ZType type)
{
    switch(type) {
        case INVALID:  return "invalid";
        case NIL:      return "nil";
        case BOOLEAN:  return "boolean";
        case NUMBER:   return "number";
        case FUNCTION: return "function";
        case STRING:   return "string";
        default:       return "undefined (?)";
    }
}

// }}}

// {{{ CODE EXECUTION

static void zoe_eq(Zoe* Z)
{
    ZType tb = zoe_gettype(Z, -1),
          ta = zoe_gettype(Z, -2);
    if(ta != tb) {
        zoe_pop(Z, 2);
        zoe_pushboolean(Z, false);
    } else {
        switch(ta) {
            case NIL:
                zoe_pop(Z, 2);
                zoe_pushboolean(Z, true);
                break;
            case BOOLEAN: {
                    bool b = zoe_popboolean(Z),
                         a = zoe_popboolean(Z);
                    zoe_pushboolean(Z, a == b);
                }
                break;
            case NUMBER: {
                    double b = zoe_popnumber(Z),
                           a = zoe_popnumber(Z);
                    zoe_pushboolean(Z, fabs(a - b) < DBL_EPSILON);
                }
                break;
            case STRING: {
                    char *b = zoe_popstring(Z),
                         *a = zoe_popstring(Z);
                    zoe_pushboolean(Z, strcmp(a, b) == 0);
                    free(a);
                    free(b);
                }
                break;
            case FUNCTION:
                zoe_error(Z, "function comparison not implemented yet"); // TODO
                abort();
            case INVALID:
            default:
                zoe_error(Z, "equality does not exists for type %s", zoe_typename(ta));
        }
    }
}


void zoe_oper(Zoe* Z, Operator oper)
{
    if(oper == ZOE_NEG) {
        zoe_pushnumber(Z, -zoe_popnumber(Z));
    } else if(oper == ZOE_NOT) {
        ZType t = zoe_peektype(Z);
        if(t == NUMBER) {
            double d = zoe_popnumber(Z);
            int64_t c = (int64_t)d;
            zoe_pushnumber(Z, ~c);
        } else if(t == BOOLEAN) {
            zoe_pushboolean(Z, !zoe_popboolean(Z));
        } else {
            zoe_error(Z, "Expected number or boolean, found %s\n", zoe_typename(t));
        }
    } else if(oper == ZOE_EQ) {
        zoe_eq(Z);
    } else {
        double b = zoe_popnumber(Z), 
               a = zoe_popnumber(Z);
        double c = 0.0;

        // TODO - should we block bitwise operations on non-integers?

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
            case ZOE_NOT:
            case ZOE_EQ:
            default:
                zoe_error(Z, "Invalid operator (code %d)", oper);
                return;
        }
        zoe_pushnumber(Z, c);
    }
}


void zoe_eval(Zoe* Z, const char* code)
{
    Bytecode* bc = bytecode_newfromcode(Z->uf, code);

    uint8_t* buffer;
    size_t sz = bytecode_generatezb(bc, &buffer);

    bytecode_free(bc);

    ZFunction f = {
        .type = BYTECODE,
        .n_args = 0,
        .bfunction = {
            .bytecode = buffer,
            .sz = sz,
        },
    };

    zoe_pushfunction(Z, f);
}


static void zoe_execute(Zoe* Z, uint8_t* data, size_t sz)
{
    Bytecode* bc = bytecode_newfromzb(Z->uf, data, sz);
    uint64_t p = 0;

    while(p < bc->code_sz) {
        Opcode op = bc->code[p];
        switch(op) {
            // 
            // push 
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
            case LT:   zoe_oper(Z, ZOE_LT);   ++p; break;
            case LTE:  zoe_oper(Z, ZOE_LTE);  ++p; break;
            case GT:   zoe_oper(Z, ZOE_GT);   ++p; break;
            case GTE:  zoe_oper(Z, ZOE_GTE);  ++p; break;
            case EQ:   zoe_oper(Z, ZOE_EQ);   ++p; break;
            default:
                zoe_error(Z, "Invalid opcode 0x%02X.", op);
        }
    }

    bytecode_free(bc);
}


void zoe_call(Zoe* Z, int n_args)
{
    STPOS initial = zoe_stacksize(Z);

    // load function
    ZFunction f = zoe_popfunction(Z);
    if(f.type != BYTECODE) {
        zoe_error(Z, "Can only execute code in ZB format.");
    }
    if(f.n_args != n_args) {
        zoe_error(Z, "Wrong number of arguments: expected %d, found %d.", f.n_args, n_args);
    }

    // execute
    zoe_execute(Z, f.bfunction.bytecode, f.bfunction.sz);

    // free
    if(f.type == BYTECODE) {
        free(f.bfunction.bytecode);
    }

    // verify if the stack has now is the same size as the beginning
    // (-1 function +1 return argument)
    if(zoe_stacksize(Z) != initial) {
        zoe_error(Z, "Function should have returned exaclty one argument.");
    }

    // remove arguments from stack
    // TODO
}


// }}}

// {{{ DEBUGGING

static int aprintf(Zoe* Z, char** ptr, const char* fmt, ...) __attribute__ ((format (printf, 3, 4)));
static int aprintf(Zoe* Z, char** ptr, const char* fmt, ...)
{
    va_list ap;

    size_t cur_sz = (*ptr ? strlen(*ptr) : 0);

    va_start(ap, fmt);
    int new_sz = cur_sz + vsnprintf(NULL, 0, fmt, ap) + 1;
    va_end(ap);
    *ptr = Z->uf->realloc(*ptr, new_sz);
    va_start(ap, fmt);
    int r = vsnprintf((*ptr) + cur_sz, new_sz, fmt, ap);
    va_end(ap);

    return r;
}

void zoe_disassemble(Zoe* Z)
{
    char* buf = NULL;

    ZFunction f = zoe_peekfunction(Z);
    if(f.type != BYTECODE) {
        zoe_error(Z, "Only bytecode functions can be disassembled.");
    }

    uint64_t p = 0;
    int ns;

#define next(sz) {                                \
    aprintf(Z, &buf, "%*s", 28-ns, " ");          \
    for(uint8_t i=0; i<sz; ++i) {                 \
        aprintf(Z, &buf, "%02X ", bc->code[p+i]); \
    }                                             \
    aprintf(Z, &buf, "\n");                       \
    p += sz;                                      \
}

    Bytecode* bc = bytecode_newfromzb(Z->uf, f.bfunction.bytecode, f.bfunction.sz);

    while(p < bc->code_sz) {
        aprintf(Z, &buf, "%08" PRIx64 ":\t", p);
        Opcode op = (Opcode)bc->code[p];
        switch(op) {
            case PUSH_Nil: ns = aprintf(Z, &buf, "PUSH_Nil") - 1; next(1); break;
            case PUSH_Bt:  ns = aprintf(Z, &buf, "PUSH_Bt") - 1; next(1); break;
            case PUSH_Bf:  ns = aprintf(Z, &buf, "PUSH_Bf") - 1; next(1); break;
            case PUSH_N: {
                    ns = aprintf(Z, &buf, "PUSH_N\t");
                    double v;
                    memcpy(&v, &bc->code[p+1], 8);

                    char nbuf[128];
                    snprintf(nbuf, sizeof nbuf, "%0.14f", v);
                    int lg;
                    while(lg = strlen(nbuf)-1, (nbuf[lg] == '0' || nbuf[lg] == '.') && lg > 0) {
                        nbuf[lg] = '\0';
                    }

                    ns += aprintf(Z, &buf, "%s", nbuf);
                    next(9);
                }
                break;
            case ADD:  ns = aprintf(Z, &buf, "ADD") - 1;  next(1); break;
            case SUB:  ns = aprintf(Z, &buf, "SUB") - 1;  next(1); break;
            case MUL:  ns = aprintf(Z, &buf, "MUL") - 1;  next(1); break;
            case DIV:  ns = aprintf(Z, &buf, "DIV") - 1;  next(1); break;
            case IDIV: ns = aprintf(Z, &buf, "IDIV") - 1; next(1); break;
            case MOD:  ns = aprintf(Z, &buf, "MOD") - 1;  next(1); break;
            case POW:  ns = aprintf(Z, &buf, "POW") - 1;  next(1); break;
            case NEG:  ns = aprintf(Z, &buf, "NEG") - 1;  next(1); break;
            case AND:  ns = aprintf(Z, &buf, "AND") - 1;  next(1); break;
            case XOR:  ns = aprintf(Z, &buf, "XOR") - 1;  next(1); break;
            case OR:   ns = aprintf(Z, &buf, "OR")  - 1;  next(1); break;
            case SHL:  ns = aprintf(Z, &buf, "SHL") - 1;  next(1); break;
            case SHR:  ns = aprintf(Z, &buf, "SHR") - 1;  next(1); break;
            case NOT:  ns = aprintf(Z, &buf, "NOT") - 1;  next(1); break;
            case LT:   ns = aprintf(Z, &buf, "LT")  - 1;  next(1); break;
            case LTE:  ns = aprintf(Z, &buf, "LTE") - 1;  next(1); break;
            case GT:   ns = aprintf(Z, &buf, "GT")  - 1;  next(1); break;
            case GTE:  ns = aprintf(Z, &buf, "GTE") - 1;  next(1); break;
            case EQ:   ns = aprintf(Z, &buf, "EQ")  - 1;  next(1); break;
            default:
                aprintf(Z, &buf, "Invalid opcode %02X\n", (uint8_t)op); ++p;
        }
    }

    bytecode_free(bc);

    if(buf) {
        zoe_pushstring(Z, buf);
    } else {
        zoe_pushstring(Z, "");
    }
    free(buf);
}


static char* zoe_escapestring(Zoe* Z, const char* s)
{
    char* buf = Z->uf->realloc(NULL, strlen(s));
    int a = 0, b = 0;
    while(s[a]) {
        if(s[a] == 13) {
            buf[b++] = '\\';
            buf[b++] = 'n';
            continue;
        } else if(s[a] == '\\' || s[a] == '\'') {
            buf[b++] = '\\';
        }

        buf[b++] = s[a++];
    }
    return buf;
}


void zoe_inspect(Zoe* Z)
{
    switch(zoe_peektype(Z)) {
        case INVALID:
            zoe_pushstring(Z, "invalid");
            break;
        case NIL:
            zoe_pushstring(Z, "nil");
            break;
        case BOOLEAN: {
                bool b = zoe_peekboolean(Z);
                zoe_pushstring(Z, b ? "true" : "false");
            }
            break;
        case NUMBER: {
                double n = zoe_peeknumber(Z);
                char buf[128];
                snprintf(buf, sizeof buf, "%0.14f", n);
                // remove zeroes at the and
                int lg;
                while(lg = strlen(buf)-1, (buf[lg] == '0' || buf[lg] == '.') && lg > 0) {
                    buf[lg] = '\0';
                }
                zoe_pushstring(Z, buf);
            }
            break;
        case FUNCTION:
            zoe_pushstring(Z, "function");
            break;
        case STRING: {
                const char* s = zoe_peekstring(Z);
                char* buf = zoe_escapestring(Z, s);
                zoe_pushstring(Z, buf);
                Z->uf->free(buf);
            }
            break;
        default: {
            zoe_error(Z, "Invalid type (code %d) in the stack.", zoe_peektype(Z));
        }
    }
}


// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
