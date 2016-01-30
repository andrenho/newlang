#ifndef LIB_ZOE_H_
#define LIB_ZOE_H_

#include <cstddef>
#include <memory>
#include <type_traits>
#include <vector>
using namespace std;

#include "lib/zvalue.h"

typedef vector<shared_ptr<ZValue>> ZStack;

class Zoe {
public:
    Zoe();

    // 
    // STACK ACCESS
    //
    template<class T> void Push(T const& t);

    template<class T> typename enable_if<is_floating_point<T>::value, T>::type Peek() const;

    template<class T> typename enable_if<is_floating_point<T>::value, T>::type Pop();

    inline ZStack const& Stack() const { return stack; }

private:
    ZStack stack = {};
};

#include "lib/zoe.inl.h"

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
