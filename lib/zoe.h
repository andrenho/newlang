#ifndef LIB_ZOE_H_
#define LIB_ZOE_H_

#include <cstddef>
#include <memory>
#include <type_traits>
#include <vector>
using namespace std;

#include "lib/zvalue.h"

typedef vector<shared_ptr<ZValue>> ZStack;
typedef int16_t STPOS;

class Zoe {
public:
    Zoe();

    // 
    // STACK ACCESS
    //
    STPOS                         AbsIndex(STPOS pos) const;
    template<typename T> void     Push(T const& t);
    ZType                         PeekType() const;
    template<typename T> T const& Peek() const;      // return a reference
    template<typename T> T        Pop();             // return a copy
    void                          Pop();             // return nothing
private:
    ZValue const& Get(STPOS idx) const;
public:

    // 
    // VM EXECUTION
    //
    void Eval(string const& code);
    void Call(int n_args);
private:
    void Execute(vector<uint8_t> const& data);
public:

    // 
    // DEBUGGING
    //
    void Inspect(STPOS pos);

    // 
    // DATA
    //
    inline ZStack const& Stack() const { return stack; }

private:
    ZStack stack = {};
};

#include "lib/zoe.inl.h"

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
