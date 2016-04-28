#ifndef VM_ZENV_HH_
#define VM_ZENV_HH_

#include "vm/ztable.hh"

class ZEnv : public ZTable {
public:
    ZEnv() : ZTable(PUB|MUT) {}

    void Clear() { _items.clear(); }
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
