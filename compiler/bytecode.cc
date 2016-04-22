#include "compiler/bytecode.h"

#include <algorithm>
#include <stdexcept>
using namespace std;

// {{{ READ/GENERATE BYTECODE

Bytecode::Bytecode(vector<uint8_t> const& from_zb)
{
}


vector<uint8_t> Bytecode::GenerateZB() const 
{
    // magic number and version
    vector<uint8_t> data = { 0x20, 0xE2, 0x0E, 0xFF, 0x01, 0x00, 0x01, 0x00 };

    // string list position
    uint64_t pos = 16 + _code.size();
    uint8_t* bytes = reinterpret_cast<uint8_t*>(&pos);
    copy(bytes, bytes+8, back_inserter(data));

    // add code
    copy(begin(_code), end(_code), back_inserter(data));

    // add strings
    for(auto const& s: _strings) {
        copy(begin(s.str), end(s.str), back_inserter(data));
        data.push_back(0);  // end of string
        uint8_t const* bytes = reinterpret_cast<uint8_t const*>(&s.hash);
        copy(bytes, bytes+8, back_inserter(data));
    }

    return data;
}

// }}}

// {{{ ADD CODE

void Bytecode::Add(Opcode op)
{
    if(op != PN8 && op != PNUM && op != PSTR && op != PARY && op != PTBL
            && op != JMP && op != BT && op != CALL) {
        _code.push_back(op);
    } else {
        throw invalid_argument("Parameter missing");
    }
}


void Bytecode::Add(Opcode op, double value)
{
    if(op == PNUM) {
        _code.push_back(op);
        uint8_t* bytes = reinterpret_cast<uint8_t*>(&value);
        copy(bytes, bytes+8, back_inserter(_code));
    } else {
        throw invalid_argument("Invalid double parameter for this opcode");
    }
}


void Bytecode::Add(Opcode op, uint8_t value)
{
    _code.push_back(op);
    if(op == PN8 || op == CALL) {
        _code.push_back(value);
    } else {
        throw invalid_argument("Invalid integer parameter for this opcode");
    }
}


void Bytecode::Add(Opcode op, uint16_t value)
{
    _code.push_back(op);
    if(op == PARY || op == PTBL) {
        uint8_t* bytes = reinterpret_cast<uint8_t*>(&value);
        copy(bytes, bytes+2, back_inserter(_code));
    } else {
        throw invalid_argument("Invalid integer parameter for this opcode");
    }
}


void Bytecode::Add(Opcode op, uint32_t value)
{
    _code.push_back(op);
    if(op == PSTR || op == JMP || op == BT) {
        uint8_t* bytes = reinterpret_cast<uint8_t*>(&value);
        copy(bytes, bytes+4, back_inserter(_code));
    } else {
        throw invalid_argument("Invalid integer parameter for this opcode");
    }
}


void Bytecode::Add(Opcode op, string const& s)
{
    if(op == PSTR) {
        size_t sz = _strings.size();
        _strings.push_back({ s, hash<string>()(s) });
        _code.push_back(op);
        uint8_t* bytes = reinterpret_cast<uint8_t*>(&sz);
        copy(bytes, bytes+4, back_inserter(_code));
    } else {
        throw invalid_argument("Invalid integer parameter for this opcode");
    }
}

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
