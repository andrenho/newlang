#ifndef LIB_EXCEPT_H_
#define LIB_EXCEPT_H_

#include <string>
#include <stdexcept>
using namespace std;

#define EXCEPTION(E)                                            \
    class E : public runtime_error {                            \
    public:                                                     \
        explicit E(string const& msg) : runtime_error(msg) {}   \
        explicit E(const char* msg) : runtime_error(msg) {}     \
    };

EXCEPTION(type_error)
EXCEPTION(stack_error)
EXCEPTION(not_implemented)

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
