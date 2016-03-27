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
    //stack.reserve(MaxStackSize);
}

// }}}

// {{{ STACK OPERATIONS

int8_t Zoe::StackSize() const 
{
    return static_cast<int8_t>(stack.size());
}


void Zoe::PushNil()
{
    CheckMaxSize(MaxStackSize-1);
    stack.emplace_back();
}


void Zoe::PushBoolean(bool b)
{
    CheckMaxSize(MaxStackSize-1);
    stack.emplace_back(b);
}


void Zoe::PushNumber(double n)
{
    CheckMaxSize(MaxStackSize-1);
    stack.emplace_back(n);
}


void Zoe::PushBFunction(int8_t n_args, vector<uint8_t>&& bytecode)
{
    CheckMaxSize(MaxStackSize-1);
    stack.emplace_back(n_args, move(bytecode));
}


void Zoe::Pop(int8_t n)
{
    for(int8_t i=0; i<n; ++i) {
        stack.pop_back();
    }
}


void Zoe::PopNil()
{
    PopAndCheckType(NIL);
}


bool Zoe::PopBoolean()
{
    return PopAndCheckType(BOOLEAN).boolean;
}


double Zoe::PopNumber()
{
    return PopAndCheckType(NUMBER).boolean;
}


ZBFunction Zoe::PopFunction()
{
    return PopAndCheckType(BFUNCTION).bfunction;
}


bool Zoe::PeekBoolean(int8_t pos) const
{
    pos = S(pos);
    CheckMinSize(pos);
    return stack[pos].boolean;
}


double Zoe::PeekNumber(int8_t pos) const
{
    pos = S(pos);
    CheckMinSize(pos);
    return stack[pos].number;
}


ZBFunction const& Zoe::PeekFunction(int8_t pos) const
{
    pos = S(pos);
    CheckMinSize(pos);
    return stack[pos].bfunction;
}


void Zoe::CheckMinSize(int8_t sz) const
{
    if(StackSize() <= sz) {
        Error("There are too few items in the stack.");
    }
}


void Zoe::CheckMaxSize(int8_t sz) const
{
    if(sz >= MaxStackSize) {
        Error("There are already too many items in the stack.");
    }
}


ZValue Zoe::PopAndCheckType(ZType t)
{
    CheckMinSize(0);
    ZValue v = move(stack.back());
    stack.pop_back();
    if(t != v.type) {
        Error("Expected %s, found %s.", Typename(t), Typename(v.type));
    }
    return move(v);
}

// }}}

/*
// {{{ STACK OPERATIONS

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
*/

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

// {{{ CODE MANAGEMENT

void Zoe::LoadCode(string const& code)
{
    Bytecode bc = Bytecode::FromCode(code);
    PushBFunction(0, bc.Data());
    //Push(make_unique<ZBytecodeFunction>(0, bc.Data()));
}

vector<uint8_t> const& Zoe::Dump(int8_t pos) const
{
    return *PeekFunction().bytecode;
}

// }}}

// {{{ CODE EXECUTION

void Zoe::Execute(vector<uint8_t> const& data)
{
    uint64_t p = 4;
    while(p < data.size()) {
        Opcode op = static_cast<Opcode>(data[p]);  // this is for receiving compiler warnings when something is missing
        switch(op) {
            case PUSH_Nil: PushNil(); ++p; break;
            case PUSH_Bt:  PushBoolean(true); ++p; break;
            case PUSH_Bf:  PushBoolean(false); ++p; break;
            case PUSH_N: {
                    int64_t m = static_cast<int64_t>(p);
                    double value;
                    copy(begin(data)+m, begin(data)+m+8, reinterpret_cast<uint8_t*>(&value));
                    PushNumber(value);
                    p += 9;
                }
                break;
            case ADD: PushNumber(PopNumber() + PopNumber()); ++p; break;
            case SUB: PushNumber(- PopNumber() + PopNumber()); ++p; break;
            case MUL: PushNumber(PopNumber() * PopNumber()); ++p; break;
            case DIV: PushNumber((1.0 / PopNumber()) * PopNumber()); ++p; break;
            case IDIV: PushNumber(static_cast<int64_t>((1.0 / PopNumber()) * PopNumber())); ++p; break;
            case MOD: {
                    double b = PopNumber(), a = PopNumber();
                    PushNumber(fmod(a, b)); ++p; break;
                    ++p;
                } break;
            case POW: {
                    double b = PopNumber(), a = PopNumber();
                    PushNumber(pow(a, b)); ++p; break;
                    ++p;
                }
                ++p;
                break;
            case NEG: PushNumber(-PopNumber()); ++p; break;
            default:
                Error("Invalid opcode 0x%2X.", op);
        }
    }
}

void Zoe::Call(int8_t n_args)
{
    auto initial_stack_size = StackSize();

    // load function
    ZBFunction f = PopFunction();

    // number of arguments is correct?
    if(f.n_args != n_args) {
        Error("Wrong number of arguments.");
    }

    // execute
    Execute(*f.bytecode);

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

ZType Zoe::Type(int8_t pos) const
{
    if(S(pos) >= stack.size()) {
        Error("Querying type of item %d when the list has only %d items", S(pos), StackSize());
    }
    return stack[S(pos)].type;
}


string Zoe::Inspect(int8_t pos) const
{
    switch(stack[S(pos)].type) {
        case ZType::NIL:
            return "nil";
        case ZType::BOOLEAN:
            return PeekBoolean(S(pos)) ? "true" : "false";
        case ZType::NUMBER: {
            char buf[100];
            snprintf(buf, 100, "%g", PeekNumber(S(pos)));
            return string(buf);
        }
        default: 
            Error("Invalid value type.");
    }
}


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


// }}}

}  // namespace Zoe

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
