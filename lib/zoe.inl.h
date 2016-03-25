#ifndef ZOE_ZOE_INL_H_
#define ZOE_ZOE_INL_H_

namespace Zoe {

template<typename T> T const* Zoe::Peek(int8_t n) const {
    if(stack.size() == 0) {
        Error("Peeking empty stack");
    }
    return dynamic_cast<T*>(stack[S(n)].get());
}

template<typename T> unique_ptr<T> Zoe::Remove(int8_t n) {
    if(S(n) >= stack.size()) {
        Error("Removing index %d when the list has only %d items", S(n), StackSize());
    }
    // cast ZValue
    unique_ptr<ZValue> r = move(stack[S(n)]);
    T* t = dynamic_cast<T*>(r.get());
    r.release();
    unique_ptr<T> s(t);
    // remove from stack
    stack.erase(stack.begin() + S(n));
    return move(s);
}

template<typename T> unique_ptr<T> Zoe::Pop() {
    if(stack.size() == 0) {
        Error("Popping empty stack");
    }
    return move(Remove<T>(static_cast<int8_t>(StackSize()-1)));
}

}

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
