#ifndef ZOE_BYTECODE_H_
#define ZOE_BYTECODE_H_

// {{{ ARCHITETURE TESTS

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error Sorry, Zoe is not yet implemented for big endian machines.
#endif
#if __SIZEOF_DOUBLE__ != 8
#error Sorry, Zoe is not yet implemented for machines where sizeof(double) != 8.
#endif

// }}}

#include <string>
#include <vector>
using namespace std;

#define ZB_VERSION_MINOR 1
#define ZB_VERSION_MAJOR 0
#define NO_ADDRESS 0xFFFFFFFF

typedef size_t Label;

struct LabelRef {
    uint64_t         address;
    vector<uint64_t> refs;
};

class Bytecode {
public:
    //
    // CONSTRUCTOR/DESTRUCTOR
    //
    Bytecode();
    Bytecode(string const& code);               // build from zoe code
    Bytecode(vector<uint8_t> const& data);      // build from bytecode
    Bytecode(uint8_t const* data, size_t sz);   //   "     "      "

    //
    // ADD TO CODE
    //
    void Add(uint8_t code);
    void AddF64(double code);
    void AddString(string const& str);

    // 
    // LABELS
    //
    Label CreateLabel();
    void  SetLabel(Label const& lbl);
    void  AddLabel(Label const& lbl);

    // 
    // LOCAL VARIABLES
    //
    void  AddVariableAssignment(string const& var, bool _mutable);
    void  AddVariable(string const& var);

    // 
    // MULTIPLE VARIABLES
    //
    void  MultivarReset();
    void  MultivarCreate(string const& var);
    void  AddMultivarCounter();
    void  AddMultivarAssignment(bool _mutable);

    // 
    // SCOPES
    //
    void  PushScope();
    void  PopScope();

    // 
    // GENERATE ZB FILE
    //
    vector<uint8_t> GenerateZB() const;

    // 
    // DATA VIEW
    //
    inline uint8_t VersionMinor() const { return version_minor; }
    inline uint8_t VersionMajor() const { return version_major; }
    inline vector<uint8_t> const& Code() const { return code; }

private:
    void AdjustLabels();

    // 
    // PRIVATE DATA
    //
    uint8_t version_minor = 0,
            version_major = 0;
    vector<uint8_t> code = {};
    vector<LabelRef> labels = {};

    // 
    // NON-COPYABLE OR MOVEABLE
    //
    Bytecode(Bytecode const &)        = delete;
    void operator=(Bytecode const &t) = delete;
    Bytecode(Bytecode &&)             = delete;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
