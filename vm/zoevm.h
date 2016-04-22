#ifndef VM_ZOEVM_H_
#define VM_ZOEVM_H_

#include <vector>
#include <memory>
using namespace std;

#include "vm/zvalue.h"

class ZoeVM {
public:
    ZoeVM();

    // 
    // stack management
    //
    ssize_t            StackAbs(ssize_t pos) const;
    ssize_t            StackSize() const;
    ZValue const&      Push(shared_ptr<ZValue> value);
    shared_ptr<ZValue> Pop();
    ZValue const*      GetPtr(ssize_t pos=-1) const;
    shared_ptr<ZValue> GetCopy(ssize_t pos=-1) const;
    // {{{ stack templates
    template<typename T> T const* GetPtr(ssize_t pos=-1) const {
        auto ptr = GetCopy(pos);
        return dynamic_cast<T const*>(ptr.get());
    }

    template<typename T> shared_ptr<T> GetCopy(ssize_t pos=-1) const {
        return dynamic_pointer_cast<T>(GetCopy(pos));
    }
    //}}}

private:
    vector<shared_ptr<ZValue>> _stack = {};
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
