#include "compiler/bytecode.hh"

#include <cassert>
#include <cstring>
#include <algorithm>
#include <stdexcept>
using namespace std;

#define NO_ADDRESS (0xFFFFFFFF)

// {{{ READ/GENERATE BYTECODE

constexpr uint8_t Bytecode::_MAGIC[];

Bytecode::Bytecode(vector<uint8_t> const& from_zb)
{
    if(from_zb.size() < 16 || !equal(begin(from_zb), begin(from_zb)+8, _MAGIC)) {
        throw runtime_error("Not a valid ZB file.");
    }

    uint64_t str_pos = *reinterpret_cast<uint64_t const*>(&from_zb[8]);
    copy(begin(from_zb) + 16, begin(from_zb) + static_cast<long>(str_pos), back_inserter(_code));

    while(str_pos < from_zb.size()) {
        string   s(reinterpret_cast<const char*>(&from_zb[str_pos]));
        str_pos += s.size() + 1;
        uint64_t h = *reinterpret_cast<uint64_t const*>(&from_zb[str_pos]);
        str_pos += 8;
        _strings.push_back({ s, h });
    }
    assert(str_pos == from_zb.size());
}


vector<uint8_t> Bytecode::GenerateZB()
{
    AdjustLabels();

    // magic number and version
    vector<uint8_t> data(begin(_MAGIC), end(_MAGIC));

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

// {{{ PARSE CODE

Bytecode::Bytecode(string const& code)
{
    extern int parse(Bytecode&, string const&);  // defined in compiler/parser.yy
    parse(*this, code);
}

// }}}

// {{{ ADD CODE

void Bytecode::Add(Opcode op)
{
    if(op != PN8 && op != PNUM && op != PSTR && op != PARY && op != PTBL
            && op != JMP && op != BT && op != CALL && op != SET) {
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
    if(op == PN8 || op == CALL || op == SET) {
        _code.push_back(value);
    } else {
        throw invalid_argument("Invalid integer parameter for this opcode");
    }
}


void Bytecode::Add(Opcode op, uint16_t value)
{
    _code.push_back(op);
    if(op == PARY) {
        uint8_t* bytes = reinterpret_cast<uint8_t*>(&value);
        copy(bytes, bytes+2, back_inserter(_code));
    } else {
        throw invalid_argument("Invalid integer parameter for this opcode");
    }
}


void Bytecode::Add(Opcode op, uint16_t value, uint8_t value2)
{
    _code.push_back(op);
    if(op == PTBL) {
        uint8_t* bytes = reinterpret_cast<uint8_t*>(&value);
        copy(bytes, bytes+2, back_inserter(_code));
        _code.push_back(value2);
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

// {{{ LABELS

Label Bytecode::CreateLabel()
{
    _labels.push_back({ NO_ADDRESS, {} });
    return _labels.size() - 1;
}


void Bytecode::SetLabel(Label const& lbl)
{
    _labels[lbl].address = _code.size();
}


void Bytecode::AddLabel(Label const& lbl)
{
    // add reference
    _labels[lbl].refs.push_back(_code.size());

    // add filler bytes
    for(int i=0; i<8; ++i) {
        _code.push_back(0);
    }
}


void Bytecode::AdjustLabels()
{
    for(auto const& label: _labels) {
        assert(label.address != NO_ADDRESS);  // Label without a corresponding address.
        for(auto const& ref: label.refs) {
            assert(ref <= (_code.size() + 8)); 
            // overwrite 8 bytes of code with address
            memcpy(&_code[ref], &label.address, 8);
        }
    }
    _labels.clear();
}

/// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
