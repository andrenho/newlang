#ifndef ZOE_ZOE_H_
#define ZOE_ZOE_H_

#include <cstdint>
#include <memory>
#include <vector>
using namespace std;

#include "zvalue.h"
#include "global.h"

namespace Zoe {

class Zoe {
public:

    Zoe();

    // CODE MANAGEMENT
    void LoadCode(string const& code);
    vector<uint8_t> const& Dump(int8_t pos=-1) const;

    // ERROR MANAGEMENT
    void Error(string s, ...) const __attribute__((noreturn));

    // STACK OPERATIONS
    int8_t  StackSize() const;
    void    Push(unique_ptr<ZValue>&& value);
    void    Insert(unique_ptr<ZValue>&& value, int8_t pos);
    template<typename T> T const* Peek(int8_t pos=-1) const;
    void    Pop(int8_t count=1);
    template<typename T> unique_ptr<T> Pop();
    void    Remove(int8_t pos=1);
    template<typename T> unique_ptr<T> Remove(int8_t pos);

    // CODE EXECUTION
    void Call(int8_t n_args);

    // INFORMATION
    ZType Type(int8_t pos=-1) const;
    string Inspect(int8_t pos=-1) const;

private:
    const size_t MaxStackSize = 20;
    vector<unique_ptr<ZValue>> stack;

    void Execute(vector<uint8_t> const& data);
    inline uint8_t S(int8_t v) const { return static_cast<uint8_t>((v >= 0) ? v : (StackSize() + v)); }
};

}

#include "zoe.inl.h"

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
