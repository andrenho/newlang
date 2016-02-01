#include "lib/zoe.h"

#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>   // TODO
#include <sstream>

#include "lib/bytecode.h"
#include "lib/opcode.h"

// {{{ CONSTRUCTOR/DESTRUCTOR

Zoe::Zoe()
{
    Push(nullptr);
}

// }}}

// {{{ STACK ACCESS

inline STPOS Zoe::AbsIndex(STPOS pos) const
{
    STPOS sz = static_cast<STPOS>(stack.size());
    STPOS i = (pos >= 0) ? pos : static_cast<STPOS>(sz + pos);
    assert(i >= 0);
    return i;
}


ZValue const& Zoe::Get(STPOS idx) const
{
    STPOS p = AbsIndex(idx);
    if(p >= static_cast<STPOS>(stack.size())) {
        throw "Index greater than stack size.";
    }
    return *stack.at(static_cast<size_t>(p));
}


shared_ptr<ZValue> Zoe::GetPtr(STPOS idx) const
{
    STPOS p = AbsIndex(idx);
    if(p >= static_cast<STPOS>(stack.size())) {
        throw "Index greater than stack size.";
    }
    return stack.at(static_cast<size_t>(p));
}


ZArray& Zoe::GetArray(STPOS idx) const
{
    auto ptr = GetPtr(idx);
    if(ptr->type != ARRAY) {
        throw "Expected array, found " + Typename(ptr->type) + ".";
    }
    return ptr->ary;
}


ZTable& Zoe::GetTable(STPOS idx) const
{
    auto ptr = GetPtr(idx);
    if(ptr->type != TABLE) {
        throw "Expected table, found " + Typename(ptr->type) + ".";
    }
    return ptr->table;
}


ZType Zoe::GetType(STPOS p) const
{
    return Get(p).type;
}


ZType Zoe::PeekType() const
{
    if(stack.empty()) {
        throw "Stack underflow.";
    }
    return stack.back()->type;
}


void Zoe::Pop(int n)
{
    if(stack.empty()) {
        throw "Stack underflow.";
    }
    for(int i=0; i<n; ++i) {
        stack.pop_back();
    }
}

void Zoe::Remove(STPOS idx)
{
    STPOS p = AbsIndex(idx);
    if(p >= static_cast<STPOS>(stack.size())) {
        throw "Index greater than stack size.";
    }
    stack.erase(begin(stack) + p);
}

// }}}

// {{{ ARRAYS

ZArray& Zoe::PushArray()
{
    auto ary_ptr = make_shared<ZValue>(ARRAY);
    stack.emplace_back(ary_ptr);
    return ary_ptr->ary;
}


void Zoe::ArrayAppend()
{
    if(GetType(-2) != ARRAY) {
        throw "Only arrays can be appended.";
    }

    GetPtr(-2)->ary.emplace_back(GetPtr(-1));
    Pop();
}


void Zoe::ArrayMul()
{
    double d = Pop<double>();
    if(d < 0) {
        throw "Arrays can only be multiplied by positive values.";
    }

    uint64_t mul = static_cast<uint64_t>(d);
    ZArray const& ary = GetArray(-1);

    // create new array
    ZArray& new_ary = PushArray();
    for(uint64_t i=0; i<mul; ++i) {
        copy(begin(ary), end(ary), back_inserter(new_ary));
    }

    Remove(-2);
}

// }}}

// {{{ TABLES

ZTable& Zoe::PushTable()
{
    auto tbl_ptr = make_shared<ZValue>(TABLE);
    stack.emplace_back(tbl_ptr);
    return tbl_ptr->table;
}


void Zoe::TableSet()
{
    ZTable& tbl = GetTable(-3);
    tbl.items[GetPtr(-2)] = GetPtr(-1);
    Pop(2);
}

// }}}

// {{{ VARIABLES

void Zoe::AddVariable(bool _mutable)
{
    variables.push_back({ GetPtr(-1), _mutable });
}


void Zoe::AddMultipleVariables(uint8_t count, bool _mutable)
{
    ZArray const& ary = GetArray(-1);
    if(ary.size() != count) {
        throw "The number of assignment elements doesn't match the number of "
              "items in the array (" + to_string(ary.size()) + " found, "
              "expected " + to_string(count) + ".";
    }

    for(auto const& item: ary) {
        stack.push_back(item);
        AddVariable(_mutable);
        stack.pop_back();
    }
}


void Zoe::PushVariableContents(uint64_t idx)
{
    stack.push_back(variables[idx].value);
}


// }}}

// {{{ CODE EXECUTION

void Zoe::Eval(string const& code)
{
    stack.emplace_back(make_shared<ZValue>(Bytecode(code).GenerateZB()));
}


void Zoe::Call(int n_args)
{
    (void) n_args;

    STPOS initial = static_cast<STPOS>(stack.size());

    // load function
    if(stack.empty()) {
        throw "Stack underflow.";
    }
    auto const& f = stack.back();
    f->ExpectType(FUNCTION);
    if(f->func.type != BYTECODE) {
        throw "Can only execute code in ZB format.";
    }
    auto data = f->func.bytecode;

    // remove function from stack
    stack.pop_back();

    // execute
    Execute(data);

    // final verification
    if(static_cast<STPOS>(stack.size()) != initial - 1) {
        throw "Function should have returned exactly one argument.";
    }
}


void Zoe::Execute(vector<uint8_t> const& data)
{
    Bytecode bc(data);
    uint64_t p = 0;

    while(p < bc.Code().size()) {
        // TODO - debugging
        Opcode op = static_cast<Opcode>(bc.Code()[p]);
        switch(op) {
            //
            // stack
            //
            case PUSH_Nil: Push(nullptr);        ++p;    break;
            case PUSH_Bt:  Push(true);           ++p;    break;
            case PUSH_Bf:  Push(false);          ++p;    break;
            case PUSH_N:   Push(bc.Get64<double>(p+1)); p += 9; break;
            case PUSH_S: {
                    string s = bc.GetString(p+1);
                    Push(s); 
                    p += s.size() + 2;
                }
                break;
            case POP:      Pop();                ++p;    break;

            //
            // operators
            //
            case ADD:    Op(ZOE_ADD);  ++p; break;
            case SUB:    Op(ZOE_SUB);  ++p; break;
            case MUL:    Op(ZOE_MUL);  ++p; break;
            case DIV:    Op(ZOE_DIV);  ++p; break;
            case IDIV:   Op(ZOE_IDIV); ++p; break;
            case MOD:    Op(ZOE_MOD);  ++p; break;
            case POW:    Op(ZOE_POW);  ++p; break;
            case NEG:    Op(ZOE_NEG);  ++p; break;
            case AND:    Op(ZOE_AND);  ++p; break;
            case OR:     Op(ZOE_OR);   ++p; break;
            case XOR:    Op(ZOE_XOR);  ++p; break;
            case SHL:    Op(ZOE_SHL);  ++p; break;
            case SHR:    Op(ZOE_SHR);  ++p; break;
            case NOT:    Op(ZOE_NOT);  ++p; break;
            case BNOT:   Op(ZOE_BNOT); ++p; break;
            case LT:     Op(ZOE_LT);   ++p; break;
            case LTE:    Op(ZOE_LTE);  ++p; break;
            case GT:     Op(ZOE_GT);   ++p; break;
            case GTE:    Op(ZOE_GTE);  ++p; break;
            case EQ:     Op(ZOE_EQ);   ++p; break;

            //
            // complex operators
            //
            case CAT:    Concat(); ++p; break;
            case LEN:    Len();    ++p; break;
            case LOOKUP: Lookup(); ++p; break;
            case SLICE:  Slice();  ++p; break;

            //
            // branches
            //
            case JMP: p = bc.Get64<uint64_t>(p+1); break;
            case Bfalse: p = Pop<bool>() ? p+9 : bc.Get64<uint64_t>(p+1); break;
            case Btrue: p = Pop<bool>() ? bc.Get64<uint64_t>(p+1) : p+9; break;

            //
            // array
            //
            case PUSHARY: PushArray();   ++p; break;
            case APPEND:  ArrayAppend(); ++p; break;

            // 
            // tables
            //
            case PUSHTBL: PushTable(); ++p; break;
            case TBLSET:  TableSet();  ++p; break;

            //
            // local variables
            //
            case ADDCNST:  AddVariable(false); ++p; break;
            case ADDMCNST: AddMultipleVariables(bc.Code()[p+1], false); p += 2; break;
            case GETLOCAL: PushVariableContents(bc.Get64<uint64_t>(p+1)); p += 9; break;

            //
            // others
            //
            case END: p = bc.Code().size(); break;
            default: {
                    stringstream s;
                    s << "Invalid opcode 0x" << setfill('0') << setw(2) << hex << static_cast<int>(op) << '.';
                    throw s.str();
                }
        }
    }
}

// }}}

// {{{ OPERATORS

void Zoe::Op(Operator op)
{
    // exceptions
    if(op == ZOE_NEG) {
        Push(-Pop<double>());
    } else if(op == ZOE_NOT) {
        Push(~static_cast<int64_t>(Pop<double>()));
    } else if(op == ZOE_BNOT) {
        Push(!Pop<bool>());
    } else if(op == ZOE_EQ) {
        bool eq = Get(-1) == Get(-2);
        Pop(2);
        Push(eq);
    } else if(op == ZOE_MUL && GetType(-2) == ARRAY) {
        ArrayMul();
    } else {
        // general case
        double b = Pop<double>(),
               a = Pop<double>();
        switch(op) {
            case ZOE_ADD:  Push(a + b); break;
            case ZOE_SUB:  Push(a - b); break;
            case ZOE_MUL:  Push(a * b); break;
            case ZOE_DIV:  Push(a / b); break;
            case ZOE_IDIV: Push(floor(a / b)); break;
            case ZOE_MOD:  Push(fmod(a, b)); break;
            case ZOE_POW:  Push(pow(a, b)); break;
            case ZOE_AND:  Push(static_cast<int64_t>(a) & static_cast<int64_t>(b)); break;
            case ZOE_XOR:  Push(static_cast<int64_t>(a) ^ static_cast<int64_t>(b)); break;
            case ZOE_OR:   Push(static_cast<int64_t>(a) | static_cast<int64_t>(b)); break;
            case ZOE_SHL:  Push(static_cast<int64_t>(a) << static_cast<int64_t>(b)); break;
            case ZOE_SHR:  Push(static_cast<int64_t>(a) >> static_cast<int64_t>(b)); break;
            
            case ZOE_LT:  Push(a < b);  return;
            case ZOE_LTE: Push(a <= b); return;
            case ZOE_GT:  Push(a > b);  return;
            case ZOE_GTE: Push(a >= b); return;

            case ZOE_NEG:  // pleases gcc
            case ZOE_EQ:
            case ZOE_NOT:
            case ZOE_BNOT:
            default:
                throw "Invalid operator code " + to_string(static_cast<int>(op)) + ".";
        }
    }
}

void Zoe::Concat()
{
    ZType t = GetType(-2);
    if(t == STRING) {
        string b = Pop<string>(),
               a = Pop<string>();
        Push(a + b);
    } else if(t == ARRAY) {
        ZArray const& a1 = GetArray(-2);
        ZArray const& a2 = GetArray(-1);
        ZArray& anew = PushArray();
        copy(begin(a1), end(a1), back_inserter(anew));
        copy(begin(a2), end(a2), back_inserter(anew));
        Remove(-3);
        Remove(-2);
    } else {
        throw "Expected string or array, found " + Typename(t) + ".";
    }
}

void Zoe::Len()
{
    Push(Get(-1).Len());
    Remove(-2);
}

void Zoe::Lookup()
{
    ZType t = GetType(-2);
    if(t == STRING) {
        int64_t i = static_cast<int64_t>(Pop<double>());
        string str = Pop<string>();
        uint64_t k = (i >= 0) ? static_cast<uint64_t>(i) : str.size() + static_cast<uint64_t>(i);
        // TODO - raise error if number is higher than the number of characters in the string
        Push(string(1, str[k]));
    } else if(t == ARRAY) {
        int64_t i = static_cast<int64_t>(Pop<double>());
        ZArray const& ary = GetArray(-1);
        uint64_t k = (i >= 0) ? static_cast<uint64_t>(i) : ary.size() + static_cast<uint64_t>(i);
        if(k >= ary.size()) {
            throw "Subscript out of range.";
        }
        stack.push_back(ary[k]);
        Remove(-2);
    } else if(t == TABLE) {
        ZTable const& tbl = GetTable(-2);
        stack.push_back(tbl.items.at(GetPtr(-1)));  // TODO - look for prototypes
        Remove(-3);
        Remove(-2);
    } else {
        throw "Expected string, array or table, found " + Typename(t) + ".";
    }
}

void Zoe::Slice()
{
    ZType t = GetType(-3);

    // find full size
    int64_t sz;
    if(t == STRING) {
        sz = static_cast<int64_t>(Get<string>(-3).size());
    } else if(t == ARRAY) {
        sz = static_cast<int64_t>(GetArray(-3).size());
    } else if(t == TABLE) {
        abort();  // TODO
    } else {
        throw "Expected string or array, found " + Typename(t) + ".";
    }

    // find start & finish
    int64_t finish = GetType(-1) == NIL ? (Pop(), sz) : static_cast<int64_t>(Pop<double>()),
            start  = static_cast<int64_t>(Pop<double>());
    start = (start >= 0) ? start : (sz + start);
    finish = (finish >= 0) ? finish : (sz + finish);

    if(start > sz || finish > sz) {
        throw "Subscript out of range";
    } else if(start >= finish) {
        throw "start <= finish";
    }

    // slice
    if(t == STRING) {
        Push(Pop<string>().substr(static_cast<size_t>(start), static_cast<size_t>(finish-start)));
    } else if(t == ARRAY) {
        ZArray& new_ary = PushArray();
        ZArray const& old_ary = GetArray(-2);
        copy(begin(old_ary)+start, begin(old_ary)+finish, back_inserter(new_ary));
        Remove(-2);
    }
}

// }}}

// {{{ DEBUGGING

void
Zoe::Inspect(STPOS pos)
{
    Push(Get(AbsIndex(pos)).Inspect());
}

// }}}


#if 0
// {{{ OLD C CODE

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
        case PUSHARY:  snprintf(buf, nbuf, "PUSHARY");  return 1;
        case APPEND:   snprintf(buf, nbuf, "APPEND");   return 1;
        case PUSHTBL:  snprintf(buf, nbuf, "PUSHTBL");  return 1;
        case TBLSET:   snprintf(buf, nbuf, "TBLSET");   return 1;
        case SLICE:    snprintf(buf, nbuf, "SLICE");    return 1;
        case ADDCNST:  snprintf(buf, nbuf, "ADDCNST");  return 1;
        case ADDMCNST: snprintf(buf, nbuf, "ADDMCNST %d", code[p+1]); return 2;
        case GETLOCAL: snprintf(buf, nbuf, "GETLOCAL"); return 1;
        case END:      snprintf(buf, nbuf, "END");      return 1;
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

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
