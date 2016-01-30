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
    template<typename T> void     Push(T const& t);
    ZType                         PeekType() const;
    template<typename T> T const& Peek() const;      // return a reference
    template<typename T> T        Pop();             // return a copy
    void                          Pop();             // return nothing

    // 
    // VM EXECUTION
    //
    void Eval(string const& code);
    void Call(int n_args);

    inline ZStack const& Stack() const { return stack; }

private:
    ZStack stack = {};

    void Execute(vector<uint8_t> const& data);
};

#include "lib/zoe.inl.h"

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
