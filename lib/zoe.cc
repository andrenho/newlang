#include "zoe.h"

#include <cstdio>
#include <cstdarg>
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
    uint64_t p = 4;
    while(p < data.size()) {
        Opcode op = static_cast<Opcode>(data[p]);  // this is for receiving compiler warnings when something is missing
        switch(op) {
            case PUSH_I:
                Push(make_unique<ZInteger>(data, p+1));
                p += 9;
                break;
            case PUSH_F:
                Push(make_unique<ZFloat>(data, p+1));
                p += 9;
                break;
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

// {{{ DEBUGGING

string Zoe::Inspect(int8_t pos) const
{
    ZValue const* value = Peek<ZValue>(pos);

    switch(value->Type) {
        case ZType::INTEGER:
            return to_string(dynamic_cast<ZInteger const*>(value)->Value());
        case ZType::FLOAT:
            return to_string(dynamic_cast<ZFloat const*>(value)->Value());
        default: 
            Error("Invalid value type.");
    }
}

// }}}

}  // namespace Zoe

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
