#include "catch.h"

#include <cstdint>
#include <vector>
using namespace std;

#include "bytecode.h"
#include "global.h"
using namespace Zoe;

TEST_CASE("Bytecode operations", "[bytecode]") {
    Bytecode bc;

    SECTION("adding data") {
        bc.Add_u8(0x80);
        REQUIRE(bc.Data() == vector<uint8_t> { 0x80 });
        bc.Add_i64(0x1234);
        vector<uint8_t> expected { 0x80, 0x34, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        REQUIRE(bc.Data() == expected);
    }

    SECTION("simple parsing") {
        auto bc = Bytecode::FromCode("34");
        vector<uint8_t> expected { 0x01, 34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        REQUIRE(bc.Data() == expected);
    }
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker
