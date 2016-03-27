#ifndef ZOE_BYTECODE_H_
#define ZOE_BYTECODE_H_

#include <cstdio>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
using namespace std;

#include "global.h"
#include "zvalue.h"

namespace Zoe {

class Bytecode {
public:
    inline void Add_u8(uint8_t value) { _data.push_back(value); }
    inline vector<uint8_t> Data() const { return move(_data); }

    void Add_f64(double value);

    // static
    static Bytecode FromCode(string const& code); 
    static void Disassemble(FILE* f, vector<uint8_t> const& data);

private:
    vector<uint8_t> _data = {};
};

}

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
