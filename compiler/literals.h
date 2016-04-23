#ifndef COMPILER_LITERALS_H_
#define COMPILER_LITERALS_H_

constexpr uint8_t operator "" _u8(unsigned long long v) { return static_cast<uint8_t>(v); }     // NOLINT runtime/int
constexpr uint16_t operator "" _u16(unsigned long long v) { return static_cast<uint16_t>(v); }  // NOLINT runtime/int
constexpr uint32_t operator "" _u32(unsigned long long v) { return static_cast<uint32_t>(v); }  // NOLINT runtime/int

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
