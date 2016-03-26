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
constexpr inline uint8_t operator"" _u8(unsigned long long v) { return uint8_t(v); }   // NOLINT(runtime/int)
constexpr inline int32_t operator"" _i32(unsigned long long v) { return int32_t(v); }  // NOLINT(runtime/int)
constexpr inline int64_t operator"" _i64(unsigned long long v) { return int64_t(v); }  // NOLINT(runtime/int)

// conversions
inline int64_t IntegerToInt64_t(vector<uint8_t> const& data, size_t pos) {
    return static_cast<int64_t>(data[pos])         | \
           static_cast<int64_t>(data[pos+1]) << 8  | \
           static_cast<int64_t>(data[pos+2]) << 16 | \
           static_cast<int64_t>(data[pos+3]) << 24 | \
           static_cast<int64_t>(data[pos+4]) << 32 | \
           static_cast<int64_t>(data[pos+5]) << 40 | \
           static_cast<int64_t>(data[pos+6]) << 48 | \
           static_cast<int64_t>(data[pos+7]) << 56;
}

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
