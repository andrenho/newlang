#ifndef ZOE_ZVALUE_H_
#define ZOE_ZVALUE_H_

#include <cstdint>
#include <memory>
#include <vector>
using namespace std;

namespace Zoe {

enum ZType { NIL, BOOLEAN, NUMBER, BFUNCTION };

struct ZBFunction { 
    int8_t n_args; 
    shared_ptr<vector<uint8_t>> bytecode;
};

struct ZValue {
    ZType type;
    union {
        bool       boolean;
        double     number;
        ZBFunction bfunction;
    };

    ZValue() : type(NIL) {}
    ZValue(bool b) : type(BOOLEAN), boolean(b) {}
    ZValue(double n) : type(NUMBER), number(n) {}
    ZValue(int8_t n_args, vector<uint8_t>&& bytecode): type(BFUNCTION), bfunction({ n_args, make_unique<vector<uint8_t>>(bytecode) }) {}
    ~ZValue() {}

    ZValue(ZValue&& v) : type(v.type) {
        switch(type) {
            case NIL: break;
            case BOOLEAN: boolean = v.boolean; break;
            case NUMBER: number = v.number; break;
            case BFUNCTION:
                bfunction.n_args = v.bfunction.n_args;
                bfunction.bytecode = move(v.bfunction.bytecode);
                break;
        }
    }

    ZValue(ZValue const&) = delete;
    void operator=(ZValue const&) = delete;
};

}

#endif

// vim: ts=4:sw=4:sts=4:expandtab
