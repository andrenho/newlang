#include "lib/zoe.h"

#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "lib/bytecode.h"
#include "lib/except.h"
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
        throw stack_error("Index greater than stack size.");
    }
    return *stack.at(static_cast<size_t>(p));
}


shared_ptr<ZValue> Zoe::GetPtr(STPOS idx) const
{
    STPOS p = AbsIndex(idx);
    if(p >= static_cast<STPOS>(stack.size())) {
        throw stack_error("Index greater than stack size.");
    }
    return stack.at(static_cast<size_t>(p));
}


ZArray& Zoe::GetArray(STPOS idx) const
{
    auto ptr = GetPtr(idx);
    if(ptr->type != ARRAY) {
        throw type_error("Expected array, found " + Typename(ptr->type) + ".");
    }
    return ptr->ary;
}


ZTable& Zoe::GetTable(STPOS idx) const
{
    auto ptr = GetPtr(idx);
    if(ptr->type != TABLE) {
        throw type_error("Expected table, found " + Typename(ptr->type) + ".");
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
        throw stack_error("Stack underflow.");
    }
    return stack.back()->type;
}


void Zoe::Pop(int n)
{
    if(stack.empty()) {
        throw stack_error("Stack underflow.");
    }
    for(int i=0; i<n; ++i) {
        stack.pop_back();
    }
}

void Zoe::Remove(STPOS idx)
{
    STPOS p = AbsIndex(idx);
    if(p >= static_cast<STPOS>(stack.size())) {
        throw stack_error("Index greater than stack size.");
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
        throw type_error("Only arrays can be appended.");
    }

    GetPtr(-2)->ary.emplace_back(GetPtr(-1));
    Pop();
}


void Zoe::ArrayMul()
{
    double d = Pop<double>();
    if(d < 0) {
        throw domain_error("Arrays can only be multiplied by positive values.");
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
        throw out_of_range("The number of assignment elements doesn't match the number of "
                           "items in the array (" + to_string(ary.size()) + " found, "
                           "expected " + to_string(static_cast<int>(count)) + ".");
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


void Zoe::SetVariableContents(uint64_t idx)
{
    if(!variables[idx]._mutable && variables[idx].value->type != NIL) {
        throw runtime_error("Trying to set a const variable");
    }

    switch(GetType(-1)) {
        case NIL:
        case BOOLEAN: 
        case NUMBER:
        case STRING:
            variables[idx].value = make_shared<ZValue>(Get(-1));
            break;
        case FUNCTION:
        case ARRAY:
        case TABLE:
            variables[idx].value = GetPtr(-1);
            break;
    }
}


// }}}

// {{{ SCOPES

void Zoe::PushScope()
{
    frame_pointers.push_back(variables.size());
}

void Zoe::PopScope()
{
    if(frame_pointers.empty()) {
        throw underflow_error("Stack underflow.");
    }

    // TODO - this can be faster
    uint64_t goal = frame_pointers.back();
    while(variables.size() > goal) {
        variables.pop_back();
    }
    frame_pointers.pop_back();
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
        throw stack_error("Stack underflow.");
    }
    auto const& f = stack.back();
    f->ExpectType(FUNCTION);
    if(f->func.type != BYTECODE) {
        throw domain_error("Can only execute code in ZB format.");
    }
    auto data = f->func.bytecode;

    // remove function from stack
    stack.pop_back();

    // execute
    Execute(data);

    // final verification
    if(static_cast<STPOS>(stack.size()) != initial - 1) {
        throw stack_error("Function should have returned exactly one argument.");
    }
}


void Zoe::Execute(vector<uint8_t> const& data)
{
    Bytecode bc(data);
    uint64_t p = 0;

    while(p < bc.Code().size()) {
#ifdef DEBUG
        stringstream trace;
        if(Tracer) {
            trace << setfill('0') << hex << uppercase;
            auto ds = DisassembleOpcode(bc, p);
            trace << setw(8) << p << ":   " << ds.desc << string(33 - ds.desc.size(), ' ');
        }
#endif
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
            case ADDVAR:   AddVariable(true);  ++p; break;
            case ADDMCNST: AddMultipleVariables(bc.Code()[p+1], false); p += 2; break;
            case ADDMVAR:  AddMultipleVariables(bc.Code()[p+1], true);  p += 2; break;
            case GETLOCAL: PushVariableContents(bc.Get64<uint64_t>(p+1)); p += 9; break;
            case SETLOCAL: SetVariableContents(bc.Get64<uint64_t>(p+1)); p += 9; break;

            // 
            // scopes
            //
            case PUSH_Sc: PushScope(); ++p; break;
            case POP_Sc:  PopScope();  ++p; break;

            //
            // others
            //
            case END: p = bc.Code().size(); break;
            default: {
                    stringstream s;
                    s << "Invalid opcode 0x" << setfill('0') << setw(2) << hex << static_cast<int>(op) << '.';
                    throw runtime_error(s.str());
                }
        }
#ifdef DEBUG
        if(Tracer) {
            trace << "< ";
            for(size_t i=0; i<stack.size(); ++i) { if(i != 0) { trace << ", "; } trace << stack[i]->Inspect(); }
            trace << " >";
            cout << trace.str() << endl;
        }
#endif
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
                throw runtime_error("Invalid operator code " + to_string(static_cast<int>(op)) + ".");
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
        throw type_error("Expected string or array, found " + Typename(t) + ".");
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
            throw out_of_range("Subscript out of range.");
        }
        stack.push_back(ary[k]);
        Remove(-2);
    } else if(t == TABLE) {
        ZTable const& tbl = GetTable(-2);
        stack.push_back(tbl.items.at(GetPtr(-1)));  // TODO - look for prototypes
        Remove(-3);
        Remove(-2);
    } else {
        throw type_error("Expected string, array or table, found " + Typename(t) + ".");
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
        throw type_error("Expected string or array, found " + Typename(t) + ".");
    }

    // find start & finish
    int64_t finish = GetType(-1) == NIL ? (Pop(), sz) : static_cast<int64_t>(Pop<double>()),
            start  = static_cast<int64_t>(Pop<double>());
    start = (start >= 0) ? start : (sz + start);
    finish = (finish >= 0) ? finish : (sz + finish);

    if(start > sz || finish > sz) {
        throw out_of_range("Subscript out of range");
    } else if(start >= finish) {
        throw out_of_range("start <= finish");
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

void Zoe::Inspect(STPOS pos)
{
    Push(Get(AbsIndex(pos)).Inspect());
}

#ifdef DEBUG

string Zoe::Disassemble(STPOS pos) const
{
    STPOS const idx = AbsIndex(pos);
    if(GetType(idx) != FUNCTION) {
        throw type_error("Not a function");
    }

    ZFunction const& f = Get(idx).func;
    if(f.type != BYTECODE) {
        throw not_implemented("For now, only bytecode functions can be disassembled.");
    }

    stringstream ss;
    ss << setfill('0') << hex << uppercase;

    const Bytecode bc(f.bytecode);
    size_t p = 0;
    while(p < bc.Code().size()) {
        Disassembly ds = DisassembleOpcode(bc, p);
        ss << setw(8) << p << ":   ";
        ss << ds.desc << string(33 - ds.desc.size(), ' ');
        for(size_t i=0; i<ds.sz; ++i) {
            ss << setw(2) << static_cast<int>(bc.Code()[p+i]) << " ";
        }
        ss << endl;
        p += ds.sz;
    }

    return ss.str();
}


Zoe::Disassembly Zoe::DisassembleOpcode(Bytecode const& bc, size_t p) const
{
    auto dbl_to_str = [](double d) {
        string s = to_string(d);
        s.erase(s.find_last_not_of("0")+1);
        s.erase(s.find_last_not_of(".")+1);
        return s;
    };

    auto to_hex = [](uint64_t v, uint8_t sz=8) {
        stringstream ss;
        ss << "0x" << setfill('0') << hex << uppercase << setw(sz) << v;
        return ss.str();
    };

#pragma GCC diagnostic ignored "-Wswitch-enum"
#pragma GCC diagnostic push
    Opcode op = static_cast<Opcode>(bc.Code()[p]);
    string s = opcode_names[op];
    switch(op) {
        case PUSH_N:    return { s + " " + dbl_to_str(bc.Get64<double>(p+1)), 9 };
        case PUSH_S:    return { s + " '" + bc.GetString(p+1) + "'", bc.GetString(p+1).size() + 2 };
        case JMP:       return { s + " " + to_hex(bc.Get64<uint64_t>(p+1)), 9 };
        case Btrue:     return { s + " " + to_hex(bc.Get64<uint64_t>(p+1)), 9 };
        case Bfalse:    return { s + " " + to_hex(bc.Get64<uint64_t>(p+1)), 9 };
        case ADDMCNST:  return { s + " " + to_string(static_cast<int>(bc.Code()[p+1])), 2 };
        case GETLOCAL:  return { s + " " + to_string(bc.Get64<uint64_t>(p+1)), 9 };
        case SETLOCAL:  return { s + " " + to_string(bc.Get64<uint64_t>(p+1)), 9 };
        default:
            if(op <= END) {
                return { s, 1 };
            } else {
                return { "Invalid opcode " + to_string(static_cast<int>(op)), 1 };
            }
    }
#pragma GCC diagnostic pop
}

#endif

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
