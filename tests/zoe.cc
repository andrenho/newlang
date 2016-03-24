#define CATCH_CONFIG_MAIN
#include "catch.h"

TEST_CASE("Basic sanity test", "[basic]") {
    REQUIRE(1 == 1);
}

// vim: ts=4:sw=4:sts=4:expandtab
