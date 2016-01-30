#include "lib/zoe.h"

#include <cassert>
#include <iomanip>
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
    STPOS i = (pos >= 0) ? pos : stack.size() + pos;
    assert(i >= 0);
    return i;
}


ZValue const& Zoe::Get(STPOS idx) const
{
    STPOS p = AbsIndex(idx);
    if(p >= stack.size()) {
        throw "Index greater than stack size.";
    }
    return *stack.at(idx);
}


ZType Zoe::PeekType() const
{
    if(stack.empty()) {
        throw "Stack underflow.";
    }
    return stack.back()->type;
}


void Zoe::Pop()
{
    if(stack.empty()) {
        throw "Stack underflow.";
    }
    stack.pop_back();
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
            case PUSH_N: Push(bc.GetF64(p+1)); p += 9; break;
            case POP:    Pop();                ++p;    break;

            //
            // others
            //
            case END: p = bc.Code().size(); break;
            default: {
                    stringstream s;
                    s << "Invalid opcode 0x" << setfill('0') << setw(2) << hex << op << '.';
                    throw s.str();
                }
        }
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

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
