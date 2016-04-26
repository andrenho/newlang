#ifndef COMPILER_BYTECODE_H_
#define COMPILER_BYTECODE_H_

#include <cassert>
#include <cstdint>
#include <stack>
#include <string>
#include <vector>
using namespace std;

#include "vm/opcode.hh"

typedef uint64_t Label;

class Bytecode {
public:
    Bytecode() : counters() {}

    // read/write code
    explicit Bytecode(vector<uint8_t> const& from_zb);
    vector<uint8_t> GenerateZB();

    // parse code
    explicit Bytecode(string const& code);

    // add to code
    void Add(Opcode op);
    void Add(Opcode op, double value);
    void Add(Opcode op, uint8_t value);
    void Add(Opcode op, uint16_t value);
    void Add(Opcode op, uint16_t value, uint8_t value2);
    void Add(Opcode op, uint32_t value);
    void Add(Opcode op, string const& s);

    // read code
    // {{{ T GetCode(uint64_t pos) const;
    template<typename T> typename enable_if<sizeof(T) == 1, T>::type GetCode(uint64_t pos) const {
        return static_cast<T>(Code().at(pos));
    }
    template<typename T> typename enable_if<sizeof(T) != 1, T>::type GetCode(uint64_t pos) const {
        assert(pos + sizeof(T) <= Code().size());
        T t = *reinterpret_cast<T const*>(&Code()[pos]);
        return t;
    }
    // }}}
    
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

    // extra data used by the parser
    stack<uint64_t> counters;

private:
    vector<uint8_t>  _code = {};
    vector<String>   _strings = {};
    vector<LabelRef> _labels = {};

    void AdjustLabels();

    constexpr static uint8_t _MAGIC[] { 0x20, 0xE2, 0x0E, 0xFF, 0x01, 0x00, 0x01, 0x00 };
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
