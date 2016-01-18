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
#ifdef DEBUG
    bool           debug_asm;
#endif
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
#ifdef DEBUG
    Z->debug_asm = false;
#endif
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
zoe_checktype(Zoe* Z, ZType type_expected, int i)
{
    ZValue value = stack_peek(Z->stack, i);
    if(value.type != type_expected) {
        zoe_error(Z, "Expected %s, found %s\n", zoe_typename(type_expected), zoe_typename(value.type));
    }
    return value;
}


void
zoe_peeknil(Zoe* Z)
{
    zoe_checktype(Z, NIL, -1);
}


bool
zoe_peekboolean(Zoe* Z)
{
    return zoe_checktype(Z, BOOLEAN, -1).boolean;
}


double
zoe_peeknumber(Zoe* Z)
{
    return zoe_checktype(Z, NUMBER, -1).number;
}


ZFunction
zoe_peekfunction(Zoe* Z)
{
    return zoe_checktype(Z, FUNCTION, -1).function;
}


const char*
zoe_peekstring(Zoe* Z)
{
    return zoe_checktype(Z, STRING, -1).string;
}


void
zoe_getnil(Zoe* Z, int i)
{
    zoe_checktype(Z, NIL, i);
}


bool
zoe_getboolean(Zoe* Z, int i)
{
    return zoe_checktype(Z, BOOLEAN, i).boolean;
}


double
zoe_getnumber(Zoe* Z, int i)
{
    return zoe_checktype(Z, NUMBER, i).number;
}


ZFunction
zoe_getfunction(Zoe* Z, int i)
{
    return zoe_checktype(Z, FUNCTION, i).function;
}


const char*
zoe_getstring(Zoe* Z, int i)
{
    return zoe_checktype(Z, STRING, i).string;
}


void
zoe_popnil(Zoe* Z)
{
    zoe_checktype(Z, NIL, -1);
    stack_pop(Z->stack);
}


bool
zoe_popboolean(Zoe* Z)
{
    bool b = zoe_checktype(Z, BOOLEAN, -1).boolean;
    stack_pop(Z->stack);
    return b;
}


double
zoe_popnumber(Zoe* Z)
{
    double n = zoe_checktype(Z, NUMBER, -1).number;
    stack_pop(Z->stack);
    return n;
}


ZFunction
zoe_popfunction(Zoe* Z)
{
    ZFunction f = zoe_checktype(Z, FUNCTION, -1).function;
    stack_pop(Z->stack);
    return f;
}


char*
zoe_popstring(Zoe* Z)
{
    char* s = zoe_checktype(Z, STRING, -1).string;
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

// {{{ MATH OPERATIONS

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
    } else if(oper == ZOE_CAT) {
        char *b = zoe_popstring(Z),
             *a = zoe_popstring(Z);
        a = Z->uf->realloc(a, strlen(a) + strlen(b) + 1);
        strcat(a, b);
        zoe_pushstring(Z, a);
        free(b);
        free(a);
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
            case ZOE_NOT:
            case ZOE_EQ:
            case ZOE_CAT:
            default:
                zoe_error(Z, "Invalid operator (code %d)", oper);
                return;
        }
        zoe_pushnumber(Z, c);
    }
}

// }}}

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


#ifdef DEBUG
static void zoe_dbgopcode(Zoe* Z, uint8_t* code, uint64_t p);
static void zoe_dbgstack(Zoe* Z);
#endif

static void zoe_execute(Zoe* Z, uint8_t* data, size_t sz)
{
    Bytecode* bc = bytecode_newfromzb(Z->uf, data, sz);
    uint64_t p = 0;

    while(p < bc->code_sz) {
#ifdef DEBUG
        if(Z->debug_asm) {
            zoe_dbgopcode(Z, bc->code, p);
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
            case LT:   zoe_oper(Z, ZOE_LT);   ++p; break;
            case LTE:  zoe_oper(Z, ZOE_LTE);  ++p; break;
            case GT:   zoe_oper(Z, ZOE_GT);   ++p; break;
            case GTE:  zoe_oper(Z, ZOE_GTE);  ++p; break;
            case EQ:   zoe_oper(Z, ZOE_EQ);   ++p; break;
            case CAT: zoe_oper(Z, ZOE_CAT); ++p; break;

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

#ifdef DEBUG

// {{{ FORMATTING

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


static char* zoe_escapestring(Zoe* Z, const char* s)
{
    char* buf = Z->uf->realloc(NULL, (strlen(s) * 2) + 3);
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

// {{{ OPCODE DISASSEMBLY

static int sprint_code(Zoe* Z, char* buf, size_t nbuf, uint8_t* code, uint64_t p) {
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
                char* sbuf = zoe_escapestring(Z, (char*)&code[p+1]);
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
        case LT:   snprintf(buf, nbuf, "LT");   return 1;
        case LTE:  snprintf(buf, nbuf, "LTE");  return 1;
        case GT:   snprintf(buf, nbuf, "GT");   return 1;
        case GTE:  snprintf(buf, nbuf, "GTE");  return 1;
        case EQ:   snprintf(buf, nbuf, "EQ");   return 1;
        case CAT: snprintf(buf, nbuf, "CAT"); return 1;
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
        case END:  snprintf(buf, nbuf, "END");  return 1;
        default:
            snprintf(buf, nbuf, "Invalid opcode %02X\n", (uint8_t)op);
    }
    return 1;
}

void zoe_disassemble(Zoe* Z)
{
    char* buf = NULL;

    ZFunction f = zoe_peekfunction(Z);
    if(f.type != BYTECODE) {
        zoe_error(Z, "Only bytecode functions can be disassembled.");
    }

    Bytecode* bc = bytecode_newfromzb(Z->uf, f.bfunction.bytecode, f.bfunction.sz);
    uint64_t p = 0;

    while(p < bc->code_sz) {
        // address
        int ns = aprintf(Z, &buf, "%08" PRIx64 ":\t", p);
        // code
        static char cbuf[128];
        int n = sprint_code(Z, cbuf, sizeof cbuf, bc->code, p);
        ns += aprintf(Z, &buf, "%s", cbuf);
        // spaces
        aprintf(Z, &buf, "%*s", 32-ns, " ");
        // hex code
        for(uint8_t i=0; i<n; ++i) {
            aprintf(Z, &buf, "%02X ", bc->code[p+i]);
        }
        aprintf(Z, &buf, "\n");

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


static void zoe_dbgopcode(Zoe* Z, uint8_t* code, uint64_t p)
{
    int ns = printf("%08" PRIx64 ":\t", p);

    char buf[128];
    sprint_code(Z, buf, sizeof buf, code, p);
    ns += printf("%s", buf);
    printf("%*s", 32-ns, " ");
}


static void zoe_dbgstack(Zoe* Z)
{
    printf("[");
    for(int i=0; i<zoe_stacksize(Z); ++i) {
        if(i != 0) {
            printf(", ");
        }
        zoe_inspect(Z, 0-1-i);
        char* buf = zoe_popstring(Z);
        printf("%s", buf);
        free(buf);
    }
    printf("]\n");
}

// }}}

#endif

// {{{ INSPECTION

void zoe_inspect(Zoe* Z, int i)
{
    switch(zoe_peektype(Z)) {
        case INVALID:
            zoe_pushstring(Z, "invalid");
            break;
        case NIL:
            zoe_pushstring(Z, "nil");
            break;
        case BOOLEAN: {
                bool b = zoe_getboolean(Z, i);
                zoe_pushstring(Z, b ? "true" : "false");
            }
            break;
        case NUMBER: {
                double n = zoe_getnumber(Z, i);
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
                const char* s = zoe_getstring(Z, i);
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

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
