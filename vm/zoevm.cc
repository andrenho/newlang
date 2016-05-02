#include "vm/zoevm.hh"

#include <list>
#include <iomanip>
#include <iostream>   // TODO
#include <sstream>
#include <stdexcept>

#include "compiler/bytecode.hh"
#include "vm/znil.hh"
#include "vm/zbool.hh"
#include "vm/znumber.hh"
#include "vm/zstring.hh"
#include "vm/zarray.hh"
#include "vm/ztable.hh"

ZoeVM::ZoeVM()
{
    _stack.push_back(make_shared<ZNil>());
}

// {{{ STACK MANAGEMENT

ssize_t ZoeVM::StackAbs(ssize_t pos) const
{
    ssize_t i = (pos >= 0) ? pos : StackSize() + pos;
    if(i < 0) {
        throw zoe_internal_error("Stack underflow");
    }
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

        static stringstream debug;
        if(Tracer) {
            debug.str("");
            string opc = b.DisassembleOpcode(p);
            debug << setfill('0') << hex << uppercase;
            debug << "* " << setw(8) << p << ":   " << opc;
            debug << string(20 - opc.size(), ' ');
        }

        switch(b.GetCode<Opcode>(p)) {

            case PNIL:
                Push(make_shared<ZNil>());
                break;

            case PBT:
                Push(make_shared<ZBool>(true));
                break;

            case PBF:
                Push(make_shared<ZBool>(false));
                break;

            case PN8:
                Push(make_shared<ZNumber>(b.GetCode<uint8_t>(p+1)));
                break;

            case PNUM: 
                Push(make_shared<ZNumber>(b.GetCode<double>(p+1)));
                break;

            case PSTR: {
                    Bytecode::String s = b.Strings().at(b.GetCode<uint32_t>(p+1));
                    Push(make_shared<ZString>(s.str, s.hash));
                }
                break;

            case PARY: {
                    uint16_t n = b.GetCode<uint16_t>(p+1);
                    auto ary = make_shared<ZArray>(end(_stack)-n, end(_stack));
                    Pop(n);
                    Push(ary);
                }
                break;

            case PTBL: {
                    uint16_t n = b.GetCode<uint16_t>(p+1);
                    auto tbl = make_shared<ZTable>(end(_stack)-(n*3)-1, end(_stack), false);
                    Pop(static_cast<uint16_t>(n*3+1));
                    Push(tbl);
                }
                break;

            case PTBX: {
                    uint16_t n = b.GetCode<uint16_t>(p+1);
                    auto tbl = make_shared<ZTable>(end(_stack)-(n*2)-1, end(_stack), true);
                    Pop(static_cast<uint16_t>(n*2+1));
                    Push(tbl);
                }
                break;

            case POP:
                Pop();
                break;

            case SET:
                GetCopy(-3)->OpSet(GetCopy(-2), GetCopy(-1), b.GetCode<TableConfig>(p+1));
                Remove(-3);
                Remove(-2);
                break;

            case GET:
                Push(GetPtr(-2)->OpGet(GetCopy(-1)));
                Remove(-3);
                Remove(-2);
                break;

            case CVAR: 
                _vars.push_back(GetCopy());
                break;

            case CMVAR:
                CreateVariables(b.GetCode<uint16_t>(p+1));
                break;

            case GVAR: {
                    uint32_t n = b.GetCode<uint32_t>(p+1);
                    if(n >= _vars.size()) {
                        throw zoe_internal_error("Variable stack overflow.");
                    }
                    Push(_vars[n]);
                }
                break;

            case SVAR: {
                    uint32_t n = b.GetCode<uint32_t>(p+1);
                    if(n >= _vars.size()) {
                        throw zoe_internal_error("Variable stack overflow.");
                    }
                    _vars[n] = GetCopy();
                }
                break;

            case PSHS:
                _scopes.push_back(static_cast<uint32_t>(_vars.size()));
                break;

            case POPS: {
                    uint32_t last = _scopes.back();
                    _scopes.pop_back();
                    assert(!_scopes.empty());
                    _vars.erase(begin(_vars) + last, end(_vars));
                }
                break;

            default:
                throw domain_error("Invalid opcode " + to_string(b.GetCode<uint8_t>(p)));
        }

        p += Bytecode::OpcodeSize(b.GetCode<Opcode>(p));

        if(Tracer) {
            debug << "< ";
            for(size_t i=0; i<_stack.size(); ++i) {
                if(i != 0) {
                    debug << ", ";
                }
                debug << _stack[i]->Inspect();
            }
            debug << " >\n";
            cout << debug.str();
        }
    }
#pragma GCC diagnostic pop
}

// }}}

// {{{ VARIABLES

void ZoeVM::CreateVariables(uint16_t n)
{
    ZArray const* ary = GetPtr<ZArray>();
    if(ary->Value().size() != n) {
        throw zoe_runtime_error("Number of declared variables (" + to_string(ary->Value().size()) +
                ") and array elements (" + to_string(n) + ")");
    }
    for(auto it = rbegin(ary->Value()); it != rend(ary->Value()); ++it) {
        _vars.push_back(*it);
    }
}

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
