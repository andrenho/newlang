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

#define ZB_VERSION_MINOR 1
#define ZB_VERSION_MAJOR 0
#define NO_ADDRESS (0xFFFFFFFF)

#include <string>
#include <vector>
using namespace std;

typedef size_t Label;

struct LabelRef {
    uint64_t         address;
    vector<uint64_t> refs;
};

struct Variable {
    string name;
    bool   _mutable;
};

class Bytecode {
public:
    //
    // CONSTRUCTOR/DESTRUCTOR
    //
    Bytecode();
    explicit Bytecode(string const& code);           // build from zoe code
    explicit Bytecode(vector<uint8_t> const& data);  // build from bytecode
    Bytecode(uint8_t const* data, size_t sz);        //   "     "      "

    //
    // ADD TO CODE
    //
    void Add(uint8_t code);
    template<typename T> void Add64(T data) {
        static_assert(sizeof(T) == 8, "use this method only for data types with 64 bits");
        uint8_t* bytes = reinterpret_cast<uint8_t*>(&data);
        copy(bytes, bytes+8, back_inserter(code));
    }
    void AddString(string const& str);

    //
    // READ CODE
    //
    template<typename T> T Get64(uint64_t pos) const {
        static_assert(sizeof(T) == 8, "use this method only for data types with 64 bits");
        uint8_t bytes[8];
        ssize_t p = static_cast<ssize_t>(pos);
        copy(begin(code)+p, begin(code)+p+8, bytes);
        return *reinterpret_cast<T*>(bytes);
    }
    string GetString(uint64_t pos) const;

    // 
    // LABELS
    //
    Label CreateLabel();
    void  SetLabel(Label const& lbl);
    void  AddLabel(Label const& lbl);

    // 
    // LOCAL VARIABLES
    //
    void  VariableAssignment(string const& var, bool _mutable);
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
    vector<uint8_t> GenerateZB();

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
    uint8_t          version_minor = 0,
                     version_major = 0;
    vector<uint8_t>  code = {};
    vector<LabelRef> labels = {};
    vector<Variable> vars = {};
    vector<string>   multivar = {};
    vector<uint64_t> frame_pointers = {};

    // 
    // NON-COPYABLE OR MOVEABLE
    //
    Bytecode(Bytecode const &)        = delete;
    void operator=(Bytecode const &t) = delete;
    Bytecode(Bytecode &&)             = delete;
};

#endif

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
