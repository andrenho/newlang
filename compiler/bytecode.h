#ifndef COMPILER_BYTECODE_H_
#define COMPILER_BYTECODE_H_

#include <cstdint>
#include <string>
#include <vector>
using namespace std;

#include "vm/opcode.h"

typedef uint64_t Label;

class Bytecode {
public:
    Bytecode() {}

    // read/write code
    Bytecode(vector<uint8_t> const& from_zb);
    vector<uint8_t> GenerateZB();

    // add to code
    void Add(Opcode op);
    void Add(Opcode op, double value);
    void Add(Opcode op, uint8_t value);
    void Add(Opcode op, uint16_t value);
    void Add(Opcode op, uint32_t value);
    void Add(Opcode op, string const& s);
    
    // labels
    struct LabelRef {
        uint64_t         address;
        vector<uint64_t> refs;
    };
    Label CreateLabel();
    void  SetLabel(Label const& lbl);
    void  AddLabel(Label const& lbl);

    // get information
    struct String {
        string   str;
        uint64_t hash;
    };
    vector<uint8_t> const& Code() const { return _code; }
    vector<String> const&  Strings() const { return _strings; }

private:
    vector<uint8_t>  _code = {};
    vector<String>   _strings = {};
    vector<LabelRef> _labels = {};

    void AdjustLabels();

    constexpr static uint8_t _MAGIC[] { 0x20, 0xE2, 0x0E, 0xFF, 0x01, 0x00, 0x01, 0x00 };
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
