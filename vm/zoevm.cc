#include "vm/zoevm.hh"

#include <cassert>
#include <list>
#include <iostream>   // TODO
#include <stdexcept>

#include "compiler/bytecode.hh"
#include "vm/znil.hh"
#include "vm/zbool.hh"
#include "vm/znumber.hh"
#include "vm/zstring.hh"
#include "vm/zarray.hh"
#include "vm/ztable.hh"

ZoeVM::ZoeVM()
    : _env(make_shared<ZTable>(static_cast<TableConfig>(PUB|MUT)))
{
    _stack.push_back(make_shared<ZNil>());
}

// {{{ STACK MANAGEMENT

ssize_t ZoeVM::StackAbs(ssize_t pos) const
{
    ssize_t i = (pos >= 0) ? pos : StackSize() + pos;
    assert(i >= 0);
    return i;
}


ssize_t ZoeVM::StackSize() const
{
    return static_cast<ssize_t>(_stack.size());
}


ZValue const& ZoeVM::Push(shared_ptr<ZValue> value)
{
    _stack.push_back(value);
    return *_stack.back().get();
}


shared_ptr<ZValue> ZoeVM::Pop()
{
    if(_stack.empty()) {
        throw underflow_error("Stack underflow");
    }
    auto last = _stack.back();
    _stack.pop_back();
    return last;
}


void ZoeVM::Pop(uint16_t n)
{
    if(_stack.size() < n) {
        throw underflow_error("Stack underflow");
    }
    _stack.resize(_stack.size() - n);
}


void ZoeVM::Remove(ssize_t pos)
{
    ssize_t i = StackAbs(pos);
    if(i >= StackSize()) {
        throw out_of_range("Stack access out of range");
    }
    _stack.erase(begin(_stack) + i);
}

ZValue const* ZoeVM::GetPtr(ssize_t pos) const
{
    ssize_t i = StackAbs(pos);
    if(i >= StackSize()) {
        throw out_of_range("Stack access out of range");
    }
    return _stack[static_cast<size_t>(i)].get();
}


shared_ptr<ZValue> ZoeVM::GetCopy(ssize_t pos) const
{
    ssize_t i = StackAbs(pos);
    if(i >= StackSize()) {
        throw out_of_range("Stack access out of range");
    }
    return _stack[static_cast<size_t>(i)];
}


ZType ZoeVM::GetType(ssize_t pos) const
{
    return GetPtr(pos)->Type();
}

// }}}

// {{{ CODE EXECUTION

void ZoeVM::ExecuteBytecode(vector<uint8_t> const& bytecode)
{
    Bytecode b(bytecode);

#pragma GCC diagnostic ignored "-Wswitch-enum"  // TODO
#pragma GCC diagnostic push
    uint64_t p = 0;
    while(p < b.Code().size()) {
        switch(b.GetCode<Opcode>(p)) {

            case PNIL:
                Push(make_shared<ZNil>());
                ++p;
                break;

            case PBT:
                Push(make_shared<ZBool>(true));
                ++p;
                break;

            case PBF:
                Push(make_shared<ZBool>(false));
                ++p;
                break;

            case PN8:
                Push(make_shared<ZNumber>(b.GetCode<uint8_t>(p+1)));
                p += 2;
                break;

            case PNUM: 
                Push(make_shared<ZNumber>(b.GetCode<double>(p+1)));
                p += 9;
                break;

            case PSTR: {
                    Bytecode::String s = b.Strings().at(b.GetCode<uint32_t>(p+1));
                    Push(make_shared<ZString>(s.str, s.hash));
                }
                p += 5;
                break;

            case PARY: {
                    uint16_t n = b.GetCode<uint16_t>(p+1);
                    auto ary = make_shared<ZArray>(end(_stack)-n, end(_stack));
                    Pop(n);
                    Push(ary);
                }
                p += 3;
                break;

            case PTBL: {
                    uint16_t n = b.GetCode<uint16_t>(p+1);
                    TableConfig tc = b.GetCode<TableConfig>(p+3);
                    auto tbl = make_shared<ZTable>(end(_stack)-(n*2), end(_stack), tc);
                    Pop(static_cast<uint16_t>(n*2));
                    Push(tbl);
                }
                p += 4;
                break;

            case PENV:
                Push(_env);
                ++p;
                break;

            case POP:
                Pop();
                ++p;
                break;

            case SET:
                GetCopy(-3)->OpSet(GetCopy(-2), GetCopy(-1), b.GetCode<TableConfig>(p+1));
                Remove(-3);
                Remove(-2);
                p += 2;
                break;

            case GET:
                Push(GetPtr(-2)->OpGet(GetCopy(-1)));
                Remove(-3);
                Remove(-2);
                ++p;
                break;

            default:
                throw domain_error("Invalid opcode " + to_string(b.GetCode<uint8_t>(p)));
        }
    }
#pragma GCC diagnostic pop
}

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
