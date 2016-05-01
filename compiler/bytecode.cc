#include "compiler/bytecode.hh"

#include <cassert>
#include <cstring>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>
using namespace std;

#include "vm/exceptions.hh"

#define NO_ADDRESS (0xFFFFFFFF)

// {{{ READ/GENERATE BYTECODE

constexpr uint8_t Bytecode::_MAGIC[];

Bytecode::Bytecode(vector<uint8_t> const& from_zb)
{
    if(from_zb.size() < 16 || !equal(begin(from_zb), begin(from_zb)+8, _MAGIC)) {
        throw runtime_error("Not a valid ZB file.");
    }

    uint64_t str_pos = *reinterpret_cast<uint64_t const*>(&from_zb[8]);
    copy(begin(from_zb) + 16, begin(from_zb) + static_cast<int64_t>(str_pos), back_inserter(_code));

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
    vector<uint8_t> data(begin(_MAGIC), end(_MAGIC));           // NOLINT - bug in linter

    // string list position
    uint64_t pos = 16 + _code.size();
    uint8_t* bytes = reinterpret_cast<uint8_t*>(&pos);
    copy(bytes, bytes+8, back_inserter(data));

    // add code
    copy(begin(_code), end(_code), back_inserter(data));

    // add strings
    for(auto const& s: _strings) {                              // NOLINT - bug in linter
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
    if(opcode_pars[op] == '0') {
        _code.push_back(op);
    } else {
        throw invalid_argument("Parameter missing for opcode " + opcode_names[op]);
    }
}


void Bytecode::Add(Opcode op, double value)
{
    if(opcode_pars[op] == 'd') {
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
    if(opcode_pars[op] == '1') {
        _code.push_back(value);
    } else {
        throw invalid_argument("Invalid integer parameter for this opcode");
    }
}


void Bytecode::Add(Opcode op, uint16_t value)
{
    _code.push_back(op);
    if(opcode_pars[op] == '2') {
        uint8_t* bytes = reinterpret_cast<uint8_t*>(&value);
        copy(bytes, bytes+2, back_inserter(_code));
    } else {
        throw invalid_argument("Invalid integer parameter for this opcode");
    }
}


void Bytecode::Add(Opcode op, uint32_t value)
{
    _code.push_back(op);
    if(opcode_pars[op] == '4') {                                                // NOLINT - bug in linter
        uint8_t* bytes = reinterpret_cast<uint8_t*>(&value);
        copy(bytes, bytes+4, back_inserter(_code));                             // NOLINT - bug in linter
    } else {
        throw invalid_argument("Invalid integer parameter for this opcode");
    }
}


void Bytecode::Add(Opcode op, string const& s)
{
    if(opcode_pars[op] == 's') {
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

// {{{ CREATE VARIABLE

void Bytecode::CreateVariable(string const& name, bool mut)
{
    for(ssize_t i = (static_cast<ssize_t>(_vars.size()) - 1); i >= _scopes.back(); --i) {
        if(_vars[static_cast<size_t>(i)].name == name) {
            throw zoe_syntax_error("Variable '" + name + "' already exists.");
        }
    }
    _vars.push_back({ name, mut });
}


uint32_t Bytecode::GetVariableIndex(string const& name, bool* mut)
{
    uint32_t i = static_cast<uint32_t>(_vars.size());
    for(auto it = rbegin(_vars); it != rend(_vars); ++it) {
        --i;
        if(it->name == name) {
            if(mut) {
                *mut = it->mut;
            }
            return i;
        }
    }

    throw zoe_syntax_error("Variable '" + name + "' not found.");
}


void Bytecode::PushScope()
{
    _scopes.push_back(static_cast<uint32_t>(_vars.size()));
}


void Bytecode::PopScope()
{
    uint32_t last = _scopes.back();
    _scopes.pop_back();
    assert(!_scopes.empty());
    _vars.erase(begin(_vars) + last, end(_vars));
}

// }}}

// {{{ DISASSEMBLER


string Bytecode::Disassemble() const
{
    stringstream ss;
    ss << setfill('0') << hex << uppercase;

    size_t pos = 0;
    while(pos < _code.size()) {
        uint8_t sz = 1;
        string op = DisassembleOpcode(pos, &sz);
        ss << setw(8) << pos << ":   " << op << "\n";
        pos += sz;
    }

    return ss.str();
}


string Bytecode::DisassembleOpcode(size_t pos, uint8_t* sz) const
{
    auto n = GetCode<uint8_t>(pos);
    stringstream ss;
    for(auto c: opcode_names[n]) {
        ss << static_cast<char>(tolower(c));
    }
    ss << string(8 - opcode_names[n].size(), ' ');
    ss << setfill('0') << hex << uppercase;
    switch(opcode_pars[n]) {
        case '0':
            if(sz) { 
                *sz = 1;
            }
            break;
        case '1':
            if(sz) {
                *sz = 2;
            }
            ss << static_cast<int>(GetCode<uint8_t>(pos+1));
            break;
        case '2':
            if(sz) {
                *sz = 3;
            }
            ss << static_cast<int>(GetCode<uint16_t>(pos+1));
            break;
        case '4':
            if(sz) {
                *sz = 5;
            }
            ss << static_cast<int>(GetCode<uint32_t>(pos+1));
            break;
        case 'd':
            if(sz) {
                *sz = 9;
            }
            ss << (GetCode<double>(pos+1));
            break;
        case 's':
            if(sz) {
                *sz = 5;
            }
            {
                String s = _strings.at(GetCode<uint32_t>(pos+1));
                ss << "'" << s.str << "'";
            }
            break;
    }

    return ss.str();
}


// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
