#ifndef ZOE_ZOE_INL_H_
#define ZOE_ZOE_INL_H_

// {{{ STACK ACCESS

template<class T> void
Zoe::Push(T const& t)
{
    stack.emplace_back(make_shared<ZValue>(t));
}


template<class T> inline typename enable_if<is_floating_point<T>::value, T>::type 
Zoe::Peek() const
{
    return 0;
}

// }}}

// {{{

template<class T> typename enable_if<is_floating_point<T>::value, T>::type 
Zoe::Pop()
{
    return 0;
}

// }}}

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
