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

    // STACK OPERATIONS
    int8_t  StackSize() const;
    /*
    void    Push(unique_ptr<ZValue>&& value);
    void    Insert(unique_ptr<ZValue>&& value, int8_t pos);
    template<typename T> T const* Peek(int8_t pos=-1) const;
    void    Pop(int8_t count=1);
    template<typename T> unique_ptr<T> Pop();
    void    Remove(int8_t pos=1);
    template<typename T> unique_ptr<T> Remove(int8_t pos);
    */
    void PushNil();
    void PushBoolean(bool b);
    void PushNumber(double n);
    void PushBFunction(int8_t n_args, vector<uint8_t>&& bytecode);

    void Pop(int8_t n=1);
    void PopNil();
    bool PopBoolean();
    double PopNumber();
    ZBFunction PopFunction();

    bool PeekBoolean(int8_t pos=-1) const;
    double PeekNumber(int8_t pos=-1) const;
    ZBFunction const& PeekFunction(int8_t pos=-1) const;

    // ERROR MANAGEMENT
    void Error(string s, ...) const __attribute__((noreturn));

    // CODE MANAGEMENT
    void LoadCode(string const& code);
    vector<uint8_t> const& Dump(int8_t pos=-1) const;

    // CODE EXECUTION
    void Call(int8_t n_args);

    // INFORMATION
    ZType Type(int8_t pos=-1) const;
    string Inspect(int8_t pos=-1) const;
    string Typename(ZType tp) const;

private:
    const size_t MaxStackSize = 20;
    vector<ZValue> stack;

    void CheckMinSize(int8_t sz) const;
    void CheckMaxSize(int8_t sz) const;
    ZValue PopAndCheckType(ZType);

    void Execute(vector<uint8_t> const& data);
    inline uint8_t S(int8_t v) const { return static_cast<uint8_t>((v >= 0) ? v : (StackSize() + v)); }
};

}

#include "zoe.inl.h"

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
