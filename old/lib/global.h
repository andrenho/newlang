/* This file contains global macros and functions, used
 * everywhere. */

#ifndef LIB_GLOBAL_H_
#define LIB_GLOBAL_H_

#include <cstdio>
#include <cstdlib>

// static assertions
static_assert(sizeof(double) == 8, "sizeof(double) != 8");
#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#  error "Non little-endian architectures are currently unsupported."
#endif

// when a function/method is not implemented, causes a error and abort
#define TODO(def) \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"") \
    def { fprintf(stderr, "\033[1;31mnot implemented: " #def "\033[0m\n"); abort(); } \
    _Pragma("GCC diagnostic pop")

// suffixes for several types
constexpr inline uint8_t operator"" _u8(unsigned long long v) { return uint8_t(v); }   // NOLINT(runtime/int)
constexpr inline int32_t operator"" _i32(unsigned long long v) { return int32_t(v); }  // NOLINT(runtime/int)
constexpr inline int64_t operator"" _i64(unsigned long long v) { return int64_t(v); }  // NOLINT(runtime/int)

// tty codes
#define TTY_GRAY   "\033[1;30m"
#define TTY_NORMAL "\033[0m"

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
