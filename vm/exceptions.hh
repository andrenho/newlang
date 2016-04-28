#ifndef VM_EXCEPTIONS_HH_
#define VM_EXCEPTIONS_HH_

#include <stdexcept>

class zoe_runtime_error : public runtime_error {
public:
    using runtime_error::runtime_error;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
