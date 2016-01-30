#ifndef ZOE_ZOE_INL_H_
#define ZOE_ZOE_INL_H_

// {{{ STACK ACCESS

template<typename T> void 
Zoe::Push(T const& t)
{
    stack.emplace_back(make_shared<ZValue>(t));
}


template<typename T> T const& 
Zoe::Peek() const
{
    if(stack.empty()) {
        throw "Stack underflow.";
    }
    return stack.back()->ValueRef<T>();
}


template<typename T> T 
Zoe::Pop() 
{
    if(stack.empty()) {
        throw "Stack underflow.";
    }

    T t = stack.back()->ValueCopy<T>();
    stack.pop_back();
    return t;
}

// }}}

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
