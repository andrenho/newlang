#include "zvalue.h"

#include <algorithm>
#include <cmath>
#include <limits>
using namespace std;

namespace Zoe {

// {{{ ZInteger

ZInteger::ZInteger(vector<uint8_t> const& data, size_t n)
    : ZInteger(static_cast<int64_t>(data[n])         | \
               static_cast<int64_t>(data[n+1]) << 8  | \
               static_cast<int64_t>(data[n+2]) << 16 | \
               static_cast<int64_t>(data[n+3]) << 24 | \
               static_cast<int64_t>(data[n+4]) << 32 | \
               static_cast<int64_t>(data[n+5]) << 40 | \
               static_cast<int64_t>(data[n+6]) << 48 | \
               static_cast<int64_t>(data[n+7]) << 56)
{
}


void ZInteger::InsertIntoVector(vector<uint8_t>& vec) const
{
    for(int i=0; i<8; ++i) {
        vec.push_back(static_cast<uint8_t>((_value >> (i*8)) & 0xFF));
    }
}

// }}}

// {{{ ZFloat

ZFloat::ZFloat(vector<uint8_t> const& data, size_t n)
    : ZFloat(0.0)
{
    int64_t m = static_cast<int64_t>(n);
    copy(begin(data)+m, begin(data)+m+8, reinterpret_cast<uint8_t*>(&_value));
}


bool ZFloat::operator==(ZFloat const& other) const
{
    return abs(Value() - other.Value()) < std::numeric_limits<double>::epsilon();
}


void ZFloat::InsertIntoVector(vector<uint8_t>& vec) const
{
    uint8_t const* ptr = reinterpret_cast<uint8_t const*>(&_value);
    for(int i=0; i<8; ++i) {
        vec.push_back(ptr[i]);
    }
}

// }}}

}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
