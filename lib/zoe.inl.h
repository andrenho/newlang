#ifndef ZOE_ZOE_INL_H_
#define ZOE_ZOE_INL_H_

// {{{ STACK ACCESS

template<class T> inline void 
Zoe::Push(T const& t)
{
    stack.emplace_back(make_shared<ZValue>(t));
}


template<class T> inline T const& 
Zoe::Peek() const
{
    if(stack.empty()) {
        throw "Stack underflow.";
    }
    return stack.back()->Value<T>();
}


template<class T> inline typename enable_if<is_same<T, nullptr_t>::value, T>::type 
Zoe::Pop()
{
    if(stack.empty()) {
        throw "Stack underflow.";
    }

    ZType type = stack.back()->type;
    if(type != NIL) {
        throw "Expected 'NIL', found '" + Typename(type) + "'.";
    }
    stack.pop_back();
    return nullptr;
}


template<class T> inline typename enable_if<!is_same<T, nullptr_t>::value, T>::type 
Zoe::Pop()
{
    if(stack.empty()) {
        throw "Stack underflow.";
    }

    T t = Peek<T>();  // copy value
    stack.pop_back();
    return t;
}

// }}}

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
