#ifndef COMPILER_BYTECODE_H_
#define COMPILER_BYTECODE_H_

#include <cstdint>
#include <string>
#include <vector>
using namespace std;

#include "vm/opcode.h"

class Bytecode {
public:
    Bytecode() {}
    Bytecode(vector<uint8_t> const& from_zb);
    
    vector<uint8_t> GenerateZB() const;

    void Add(Opcode op);
    void Add(Opcode op, double value);
    void Add(Opcode op, uint8_t value);
    void Add(Opcode op, uint16_t value);
    void Add(Opcode op, uint32_t value);
    void Add(Opcode op, string const& s);

    struct String {
        string   str;
        uint64_t hash;
    };
    vector<uint8_t> const& Code() const { return _code; }
    vector<String> const&  Strings() const { return _strings; }

private:
    vector<uint8_t> _code = {};
    vector<String>  _strings = {};
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
