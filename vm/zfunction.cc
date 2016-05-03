#include "vm/zfunction.hh"

#include <sstream>
using namespace std;

uint64_t ZFunctionPointer::Value() const
{
    return _ptr;
}


uint64_t ZFunctionPointer::Hash() const
{
    return hash<uint64_t>()(_ptr);
}


bool ZFunctionPointer::OpEq(shared_ptr<ZValue> other) const
{
    if(Type() != other->Type()) {
        return false;
    }
    return _ptr == static_pointer_cast<ZFunctionPointer>(other)->_ptr;
}


string ZFunctionPointer::Inspect() const
{
    stringstream ss;
    ss << "function: 0x" << hex << uppercase << _ptr;
    return ss.str();
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
