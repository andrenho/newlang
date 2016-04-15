#include "lib/bytecode.h"

#include <cassert>
#include <cinttypes>
#include <cstring>

#include <algorithm>
#include <iterator>

#include "lib/opcode.h"

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


void Bytecode::AddString(string const& str)
{
    copy(begin(str), end(str), back_inserter(code));
    Add(0);
}

// }}}

// {{{ READ CODE

string Bytecode::GetString(uint64_t pos) const
{
    return string(reinterpret_cast<const char*>(&code[pos]));
}

// }}}

// {{{ LABELS

Label Bytecode::CreateLabel()
{
    labels.push_back({ NO_ADDRESS, {} });
    return labels.size() - 1;
}


void Bytecode::SetLabel(Label const& lbl)
{
    labels[lbl].address = code.size();
}


void Bytecode::AddLabel(Label const& lbl)
{
    // add reference
    labels[lbl].refs.push_back(code.size());

    // add filler bytes
    for(int i=0; i<8; ++i) {
        Add(0);
    }
}

void Bytecode::AdjustLabels()
{
    for(auto const& label: labels) {
        assert(label.address != NO_ADDRESS);  // Label without a corresponding address.
        for(auto const& ref: label.refs) {
            assert(ref <= (code.size() + 8)); 
            // overwrite 8 bytes of code with address
            memcpy(&code[ref], &label.address, 8);
        }
    }
}


// }}}

// {{{ LOCAL VARIABLES

void Bytecode::VariableAssignment(string const& varname, bool _mutable)
{
    vars.push_back({ varname, _mutable });
}


void Bytecode::AddVariable(string const& varname)
{
    for(ssize_t j=static_cast<ssize_t>(vars.size())-1; j >= 0; --j) {
        if(vars[static_cast<size_t>(j)].name == varname) {
            Add(GETLOCAL);
            Add64<uint64_t>(static_cast<uint64_t>(j));
            return;
        }
    }
    throw "Variable '" + varname + "' not found.";
}

// }}}

// {{{ MULTIPLE VARIABLES

void Bytecode::MultivarReset()
{
    multivar.clear();
}


void Bytecode::MultivarCreate(string const& var)
{
    multivar.push_back(var);
}


void Bytecode::AddMultivarCounter()
{
    if(multivar.size() > 255) {
        throw "There can only be 255 multivars.";
    }
    Add(static_cast<uint8_t>(multivar.size()));
}


void Bytecode::AddMultivarAssignment(bool _mutable)
{
    for(auto const& mv: multivar) {
        VariableAssignment(mv, _mutable);
    }
}

// }}}

// {{{ SCOPES

void Bytecode::PushScope()
{
    frame_pointers.push_back(vars.size());
    Add(PUSH_Sc);
}


void Bytecode::PopScope()
{
    if(frame_pointers.empty()) {
        throw "Stack underflow.";
    }

    uint64_t goal = frame_pointers.back();
    while(vars.size() > goal) {
        vars.pop_back();
    }
    frame_pointers.pop_back();

    Add(POP_Sc);
}

// }}}

// {{{ GENERATE ZB FILE

vector<uint8_t> Bytecode::GenerateZB()
{
    // adjust labels
    AdjustLabels();

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
