#include "zoe.h"

#include <cmath>
#include <cstdio>
#include <cstdarg>
#include <functional>
#include <memory>
using namespace std;

#include "bytecode.h"
#include "opcode.h"

namespace Zoe {

// {{{ CONSTRUCTOR/DESTRUCTOR

Zoe::Zoe()
    : stack()
{
    stack.reserve(MaxStackSize);
}

// }}}

// {{{ CODE MANAGEMENT

void Zoe::LoadCode(string const& code)
{
    Bytecode bc = Bytecode::FromCode(code);
    Push(make_unique<ZBytecodeFunction>(0, bc.Data()));
}

vector<uint8_t> const& Zoe::Dump(int8_t pos) const
{
    ZBytecodeFunction const* f;
    if((f = Peek<ZBytecodeFunction>(pos))) {
        return f->Bytecode();
    } else {
        Error("Expected function");
    }
}

// }}}

// {{{ ERROR MANAGEMENT

void Zoe::Error(string s, ...) const
{
    va_list ap;
    va_start(ap, s);
    fprintf(stderr, "zoe: ");
    vfprintf(stderr, s.c_str(), ap);
    fprintf(stderr, "\n");
    abort();
}

// }}}

// {{{ STACK OPERATIONS

int8_t Zoe::StackSize() const 
{
    return static_cast<int8_t>(stack.size());
}


void Zoe::Push(unique_ptr<ZValue>&& value)
{
    if(stack.size() == MaxStackSize) {
        Error("Stack overflow");  // TODO - improve message
    }
    stack.push_back(move(value));
}


void Zoe::Insert(unique_ptr<ZValue>&& value, int8_t pos)
{
    if(stack.size() == MaxStackSize) {
        Error("Stack overflow");  // TODO - improve message
    }
    stack.insert(stack.begin() + pos, move(value));
}


void Zoe::Pop(int8_t count)
{
    if(stack.size() == 0) {
        Error("Popping empty stack");
    }
    for(int8_t i=0; i<count; ++i) {
        Remove(static_cast<int8_t>(StackSize()-1));
    }
}


void Zoe::Remove(int8_t pos)
{
    if(S(pos) >= stack.size()) {
        Error("Removing index %d when the list has only %d items", S(pos), StackSize());
    }
    stack.erase(stack.begin() + S(pos));
}


// }}}

// {{{ CODE EXECUTION

void Zoe::Execute(vector<uint8_t> const& data)
{
    // {{{ lambda: math
    auto do_math_op = [this](function<double(double, double)> const& f) {
        ZType tp_b = Type(-1),
              tp_a = Type(-2);
        auto b = Pop<ZNumber>();
        auto a = Pop<ZNumber>();
        if(a && b) {
            Push(make_unique<ZNumber>(f(a->Value(), b->Value())));
        } else {
            Error("Can't sum " + Typename(tp_a) + " and " + Typename(tp_b) + ".");
        }
    };
    // }}}

    uint64_t p = 4;
    while(p < data.size()) {
        Opcode op = static_cast<Opcode>(data[p]);  // this is for receiving compiler warnings when something is missing
        switch(op) {
            case PUSH_Nil:
                Push(make_unique<ZNil>());
                ++p;
                break;
            case PUSH_Bt:
                Push(make_unique<ZBoolean>(true));
                ++p;
                break;
            case PUSH_Bf:
                Push(make_unique<ZBoolean>(false));
                ++p;
                break;
            case PUSH_N:
                Push(make_unique<ZNumber>(data, p+1));
                p += 9;
                break;
            case ADD:
                do_math_op([](double a, double b) { return a+b; });
                ++p;
                break;
            case SUB:
                do_math_op([](double a, double b) { return a-b; });
                ++p;
                break;
            case MUL:
                do_math_op([](double a, double b) { return a*b; });
                ++p;
                break;
            case DIV:
                do_math_op([](double a, double b) { return a/b; });
                ++p;
                break;
            case IDIV:
                do_math_op([](double a, double b) { return static_cast<int64_t>(a/b); });
                ++p;
                break;
            case MOD:
                do_math_op([](double a, double b) { return fmod(a, b); });
                ++p;
                break;
            case POW:
                do_math_op([](double a, double b) { return pow(a, b); });
                ++p;
                break;
            case NEG: {
                    ZType tp = Type(-1);
                    if(tp != NUMBER) {
                        Error("Can't negate " + Typename(tp) + ".");
                    }
                    Push(make_unique<ZNumber>(-Pop<ZNumber>()->Value()));
                    ++p;
                } break;
            default:
                Error("Invalid opcode 0x%2X.", op);
        }
    }
}

void Zoe::Call(int8_t n_args)
{
    auto initial_stack_size = StackSize();

    // load function
    unique_ptr<ZBytecodeFunction> f;
    if(!(f = Pop<ZBytecodeFunction>())) {
        Error("Element being called is not a function.");
    }

    // number of arguments is correct?
    if(f->NArgs() != n_args) {
        Error("Wrong number of arguments.");
    }

    // execute
    Execute(f->Bytecode());

    // verify if the stack has now is the same size as the beginning
    // (-1 function +1 return argument)
    if(StackSize() != initial_stack_size) {
        Error("Function should have returned exaclty one argument.");
    }

    // remove arguments from stack
    // TODO
}

// }}}

// {{{ INFORMATION

string Zoe::Typename(ZType tp) const
{
    switch(tp) {
        case NIL:       return "nil";
        case BOOLEAN:   return "boolean";
        case NUMBER:    return "number";
        case BFUNCTION: return "function";
    }
    abort();
}


ZType Zoe::Type(int8_t pos) const
{
    if(S(pos) >= stack.size()) {
        Error("Querying type of item %d when the list has only %d items", S(pos), StackSize());
    }
    return stack[S(pos)]->Type;
}


string Zoe::Inspect(int8_t pos) const
{
    ZValue const* value = Peek<ZValue>(pos);

    switch(value->Type) {
        case ZType::NIL:
            return "nil";
        case ZType::BOOLEAN:
            return dynamic_cast<ZBoolean const*>(value)->Value() ? "true" : "false";
        case ZType::NUMBER: {
            char buf[100];
            snprintf(buf, 100, "%g", dynamic_cast<ZNumber const*>(value)->Value());
            return string(buf);
        }
        default: 
            Error("Invalid value type.");
    }
}

// }}}

}  // namespace Zoe

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
