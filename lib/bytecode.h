#ifndef ZOE_BYTECODE_H_
#define ZOE_BYTECODE_H_

#include <cstdio>
#include <cstdint>
#include <string>
#include <vector>
using namespace std;

#include "global.h"

namespace Zoe {

class Bytecode {
public:
    inline void Add_u8(uint8_t value) { _data.push_back(value); }
    inline vector<uint8_t> const& Data() const { return _data; }

    void Add_i64(int64_t value);

    // static
    static Bytecode FromCode(string const& code); 
    static void Disassemble(FILE* f, vector<uint8_t> const& data);

private:
    vector<uint8_t> _data = {};
};

}

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
