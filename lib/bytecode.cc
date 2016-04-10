#include "lib/bytecode.h"

#include <algorithm>
#include <iterator>

// {{{ CONSTRUCTOR/DESTRUCTOR

Bytecode::Bytecode()
{
}


extern int parse(Bytecode&, string const&);  // defined in parser.y

Bytecode::Bytecode(string const& code)
{
    parse(*this, code);
}


Bytecode::Bytecode(vector<uint8_t> const& data)
{
    // validate magic code
    vector<uint8_t> magic = { 0x90, 0x6F, 0x65, 0x20, 0xEB, 0x00 };
    if(data.size() < 0x38 || !equal(begin(magic), end(magic), begin(data))) {
        throw "Not a ZB file.";
    }

    // validate version
    if(data[6] != 1 || data[7] != 0) {
        throw "ZB version unsupported";
    }

    // load public fields
    version_minor = data[6];
    version_major = data[7];
    copy(begin(data) + 0x38, end(data) /* TODO */, back_inserter(code));
}


Bytecode::Bytecode(uint8_t const* data, size_t sz)
    : Bytecode(vector<uint8_t>(data, data + sz))
{
}


// }}}

// {{{ ADD TO CODE

void Bytecode::Add(uint8_t data)
{
    code.push_back(data);
}


void Bytecode::AddF64(double data)
{
    uint8_t* bytes = reinterpret_cast<uint8_t*>(&data);
    copy(bytes, bytes+8, back_inserter(code));
}


void Bytecode::AddString(string const& str)
{
    copy(begin(str), end(str), back_inserter(code));
}

// }}}

// {{{ LABELS

Label Bytecode::CreateLabel()
{
    return 0;
}


Label Bytecode::SetLabel(Label const& lbl)
{
    return 0;
}


void Bytecode::AddLabel(Label const& lbl)
{
}

// }}}

// {{{ LOCAL VARIABLES

void Bytecode::AddVariableAssignment(string const& var, bool _mutable)
{
}


void Bytecode::AddVariable(string const& var)
{
}

// }}}

// {{{ MULTIPLE VARIABLES

void Bytecode::MultivarReset()
{
}


void Bytecode::MultivarCreate(string const& var)
{
}


void Bytecode::AddMultivarCounter()
{
}


void Bytecode::AddMultivarAssignment(bool _mutable)
{
}

// }}}

// {{{ SCOPES

void Bytecode::PushScope()
{
}


void Bytecode::PopScope()
{
}

// }}}

// {{{ GENERATE ZB FILE

vector<uint8_t> Bytecode::GenerateZB() const
{
    // TODO - adjust labels

    // headers
    vector<uint8_t> data = {
        0x90, 0x6F, 0x65, 0x20, 0xEB, 0x00, ZB_VERSION_MINOR, ZB_VERSION_MAJOR,
    };

    // indices
    uint64_t idx[] = {
        0x38,           // code_pos
        code.size(),    // code_sz
        0x0,            // data_pos  TODO
        0x0,            // data_sz   TODO
        0x0,            // debug_pos TODO
        0x0,            // debug_sz  TODO
    };
    for(int i=0; i<6; ++i) {
        uint8_t* bytes = reinterpret_cast<uint8_t*>(&idx[i]);
        copy(bytes, bytes+8, back_inserter(data));
    }

    // add code
    copy(begin(code), end(code), back_inserter(data));
    // TODO - others

    return data;
}

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp