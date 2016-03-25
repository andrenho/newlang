#ifndef ZOE_BYTECODE_H_
#define ZOE_BYTECODE_H_

#include <cstdint>
#include <string>
#include <vector>
using namespace std;

#include "global.h"

namespace Zoe {

class Bytecode {
public:
    inline void Add(uint8_t value) { _data.push_back(value); }
    inline vector<uint8_t> const& Data() const { return _data; }

    void Add(int32_t value);
    void Add(int64_t value);

    // static
    TODO( static Bytecode FromCode(string const& code) )

private:
    vector<uint8_t> _data = {};
};

}

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
