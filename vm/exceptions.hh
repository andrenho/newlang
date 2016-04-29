#ifndef VM_EXCEPTIONS_HH_
#define VM_EXCEPTIONS_HH_

#include <stdexcept>
using namespace std;

class zoe_runtime_error : public logic_error {
public:
    using logic_error::logic_error;
};

class zoe_syntax_error : public runtime_error {
public:
    using runtime_error::runtime_error;
};

class zoe_internal_error : public runtime_error {
public:
    using runtime_error::runtime_error;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
