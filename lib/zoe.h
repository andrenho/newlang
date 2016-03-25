#ifndef ZOE_ZOE_H_
#define ZOE_ZOE_H_

#include <cstdint>
#include <memory>
#include <vector>
using namespace std;

#include "zvalue.h"

namespace Zoe {

class Zoe {
public:

    Zoe();

    void Error(string s, ...) const __attribute__((noreturn));

    //
    // STACK OPERATION
    //
    int8_t  StackSize() const;
    void    Push(unique_ptr<ZValue>&& value);
    void    Insert(unique_ptr<ZValue>&& value, int8_t pos);
    template<typename T> T const* Peek(int8_t pos=-1) const;
    void    Pop(int8_t count=1);
    template<typename T> unique_ptr<T> Pop();
    void    Remove(int8_t pos=1);
    template<typename T> unique_ptr<T> Remove(int8_t pos);

private:
    const size_t MaxStackSize = 20;
    vector<unique_ptr<ZValue>> stack;

    inline uint8_t S(int8_t v) const { return static_cast<uint8_t>((v >= 0) ? v : (StackSize() + v)); }
};

}

#include "zoe.inl.h"

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
