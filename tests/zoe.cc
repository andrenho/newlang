#include "catch.h"

#include <memory>
using namespace std;

#include "zoe.h"
using namespace Zoe;

TEST_CASE("Zoe stack operations", "[zoe]") {
    Zoe::Zoe zoe;

    REQUIRE(zoe.StackSize() == 0);

    SECTION("push, peek and pop") {
        zoe.Push(make_unique<ZInteger>(24));
        REQUIRE(zoe.StackSize() == 1);
        REQUIRE(zoe.Peek<ZInteger>()->Value() == 24);
        REQUIRE(zoe.Pop<ZInteger>()->Value() == 24);
        REQUIRE(zoe.StackSize() == 0);
    }

    SECTION("peek in the middle, remove in the middle") {
        zoe.Push(make_unique<ZInteger>(1));
        zoe.Push(make_unique<ZInteger>(2));
        zoe.Push(make_unique<ZInteger>(3));
        REQUIRE(zoe.StackSize() == 3);
        REQUIRE(zoe.Peek<ZInteger>(1)->Value() == 2);
        REQUIRE(zoe.Peek<ZInteger>(-2)->Value() == 2);
        REQUIRE(zoe.Remove<ZInteger>(-2)->Value() == 2);
        REQUIRE(zoe.Peek<ZInteger>(0)->Value() == 1);
        REQUIRE(zoe.Peek<ZInteger>(1)->Value() == 3);
        REQUIRE(zoe.StackSize() == 2);
    }

    SECTION("remove generic items") {
        zoe.Push(make_unique<ZInteger>(1));
        zoe.Push(make_unique<ZInteger>(2));
        zoe.Push(make_unique<ZInteger>(3));
        zoe.Push(make_unique<ZInteger>(4));
        zoe.Pop(2);
        REQUIRE(zoe.StackSize() == 2);
        zoe.Remove(0);
        REQUIRE(zoe.StackSize() == 1);
        REQUIRE(zoe.Peek<ZInteger>(0)->Value() == 2);
    }

    SECTION("insert items") {
        zoe.Push(make_unique<ZInteger>(1));
        zoe.Push(make_unique<ZInteger>(3));
        zoe.Insert(make_unique<ZInteger>(4), 1);
        REQUIRE(zoe.StackSize() == 3);
        REQUIRE(zoe.Peek<ZInteger>(1)->Value() == 4);
    }
}

// vim: ts=4:sw=4:sts=4:expandtab
