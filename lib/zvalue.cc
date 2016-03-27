#include "zvalue.h"

#include <algorithm>
#include <cmath>
#include <limits>
using namespace std;

namespace Zoe {

// {{{ ZNumber
/*
ZNumber::ZNumber(vector<uint8_t> const& data, size_t n)
    : ZNumber(0.0)
{
    int64_t m = static_cast<int64_t>(n);
    copy(begin(data)+m, begin(data)+m+8, reinterpret_cast<uint8_t*>(&_value));
}


bool ZNumber::operator==(ZNumber const& other) const
{
    return abs(Value() - other.Value()) < std::numeric_limits<double>::epsilon();
}


void ZNumber::InsertIntoVector(vector<uint8_t>& vec) const
{
    uint8_t const* ptr = reinterpret_cast<uint8_t const*>(&_value);
    for(int i=0; i<8; ++i) {
        vec.push_back(ptr[i]);
    }
}
*/
// }}}

}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
