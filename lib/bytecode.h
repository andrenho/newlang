#ifndef LIB_BYTECODE_H_
#define LIB_BYTECODE_H_

#include <cstdint>
#include <string>
#include <vector>
using namespace std;

class Bytecode {
public:
    Bytecode() {}
    Bytecode(vector<uint8_t> const& from_zb);
    
    vector<uint8_t> GenerateZB() const;

    vector<uint8_t> const& Code() const { return _code; }
    vector<string> const&  Strings() const { return _strings; }

private:
    vector<uint8_t> _code = {};
    vector<string>  _strings = {};
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
