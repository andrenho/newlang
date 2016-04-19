#ifndef LIB_ZOE_H_
#define LIB_ZOE_H_

#include <cstddef>
#include <memory>
#include <type_traits>
#include <vector>
using namespace std;

#include "lib/zvalue.h"

typedef vector<shared_ptr<ZValue>> ZStack;
typedef int16_t STPOS;

class Zoe {
public:
    Zoe();

    // 
    // STACK ACCESS
    //
    STPOS                         AbsIndex(STPOS pos) const;
    template<typename T> T const& Get(STPOS pos) const;
    ZType                         GetType(STPOS pos) const;
    template<typename T> void     Push(T const& t);
    ZType                         PeekType() const;
    template<typename T> T const& Peek() const;      // return a reference
    template<typename T> T        Pop();             // return a copy
    void                          Pop(int n=1);      // return nothing
    void                          Remove(STPOS pos);
private:
    ZValue const& Get(STPOS idx) const;
    shared_ptr<ZValue> GetPtr(STPOS idx) const;
    ZArray& GetArray(STPOS idx) const;
    ZTable& GetTable(STPOS idx) const;

    ZStack stack = {};
public:
    inline ZStack const& Stack() const { return stack; }

    //
    // ARRAY
    //
    ZArray& PushArray();
    void ArrayAppend();
private:
    void ArrayMul();
public:

    // 
    // TABLES
    //
    ZTable& PushTable();
    void    TableSet();

    // 
    // VARIABLES
    //
    void AddVariable(bool _mutable);
    void AddMultipleVariables(uint8_t count, bool _mutable);
    void PushVariableContents(uint64_t idx);
    void SetVariableContents(uint64_t idx);
private:
    struct Variable {
        shared_ptr<ZValue> value;
        bool               _mutable;
    };

    vector<uint64_t> frame_pointers = {};
    vector<Variable> variables = {};
public:

    //
    // SCOPES
    //
    void PushScope();
    void PopScope();

    // 
    // VM EXECUTION
    //
    enum Operator : uint8_t {
        ZOE_ADD, ZOE_SUB, ZOE_MUL, ZOE_DIV, ZOE_IDIV, ZOE_MOD, ZOE_POW, ZOE_NEG,    // arithmetic
        ZOE_AND, ZOE_XOR, ZOE_OR, ZOE_SHL, ZOE_SHR, ZOE_NOT,                        // bitwise
        ZOE_LT, ZOE_LTE, ZOE_GT, ZOE_GTE, ZOE_EQ,                                   // relational
        ZOE_BNOT,                                                                   // boolean
    };

    void Eval(string const& code);
    void Call(int n_args);
    void Op(Operator op);
    void Concat();
    void Len();
    void Lookup();
    void Setup();
    void Slice();
private:
    void Execute(vector<uint8_t> const& data);
public:

    // 
    // DEBUGGING
    //
    void Inspect(STPOS pos);
#ifdef DEBUG
    bool Tracer = false;

    string Disassemble(STPOS pos) const;
private:
    struct Disassembly { 
        string desc; 
        size_t sz; 
    };  
    Disassembly DisassembleOpcode(class Bytecode const& bc, size_t p) const;
#endif
};

#include "lib/zoe.inl.h"

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
