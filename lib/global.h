/* This file contains global macros and functions, used
 * everywhere. */

#ifndef LIB_GLOBAL_H_
#define LIB_GLOBAL_H_

#include <cstdio>
#include <cstdlib>

// when a function/method is not implemented, causes a error and abort
#define TODO(def) \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"") \
    def { fprintf(stderr, "\033[1;31mnot implemented: " #def "\033[0m\n"); abort(); } \
    _Pragma("GCC diagnostic pop")

// suffixes for several types
constexpr inline uint8_t operator"" _u8(unsigned long long v) { return uint8_t(v); }
constexpr inline int32_t operator"" _i32(unsigned long long v) { return int32_t(v); }
constexpr inline int64_t operator"" _i64(unsigned long long v) { return int64_t(v); }

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
