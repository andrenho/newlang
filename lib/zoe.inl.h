#ifndef ZOE_ZOE_INL_H_
#define ZOE_ZOE_INL_H_

// {{{ STACK ACCESS

template<class T> void
Zoe::Push(T const& t)
{
    stack.emplace_back(make_shared<ZValue>(t));
}


template<class T> T const& 
Zoe::Peek() const
{
    if(stack.empty()) {
        throw "Stack underflow.";
    }
    return stack.back()->Value<T>();
}


template<class T> T
Zoe::Pop()
{
    T t = Peek<T>();  // copy value
    stack.pop_back();
    return t;
}

// }}}

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
