#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>
using namespace std;

#include "compiler/bytecode.hh"
#include "compiler/literals.hh"
#include "vm/zoevm.hh"
#include "vm/znil.hh"
#include "vm/zbool.hh"
#include "vm/znumber.hh"
#include "vm/zstring.hh"
#include "vm/zarray.hh"
#include "vm/ztable.hh"

// {{{ TEST INFRASTRUCTURE

#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wsign-promo"

static int tests_run = 0;
static string current_test;  // NOLINT runtime/string


//
// test list
//
struct Test {
    string desc;
    function<void()> f;
};
static vector<Test> test_list;

static void _run_test(string const& desc, function<void()> f)
{
    test_list.push_back({ desc, f });
}
#define run_test(test) _run_test(#test, test)


// 
// util functions
//
template<typename T> string to_string(vector<T> const& v) {
    string s = "[";
    for(size_t i=0; i<v.size(); ++i) {
        if(i != 0) {
            s.append(", ");
        }
        s.append(to_string(v[i]));
    }
    return s + "]";
}

static string to_string(string const& s) {
    return s;
}


//
// basic assertions
//
template<typename T> static void _mequals(string const& code, function<T()> const& f, T const& expected, string const& message="")
{
    ++tests_run;
    string m = string("[ ") + current_test + " ] " + ((message != "") ? message : code);
    T result;
    try {
        result = f();
        if(result != expected) {
            cout << "not ok " << tests_run << " - " << m << " (found " << to_string(result) << ", expected " << to_string(expected) << ")\n";
        } else {
            cout << "ok " << tests_run << " - " << m << " == " << to_string(expected) << "\n";
        }
    } catch(exception const& e) {
        cout << "not ok " << tests_run << " - " << m << " (exception thrown: " << e.what() << ")\n";
    }
}

template<typename T> static void _mequals(string const& code, function<string()> const& f, const char* expected, string const& message="")
{
    _mequals<string>(code, f, string(expected), message);
}

#define mequals(code, expected, ...) _mequals<decltype(expected)>(string(#code), [&]() { return code; }, expected, ##__VA_ARGS__)


static void _mthrows(string const& code, function<void()> const& f, string const& message="")
{
    ++tests_run;
    string m = string("[ ") + current_test + " ] " + ((message != "") ? message : code);
    try {
        f();
        cout << "not ok " << tests_run << " - " << m << " (expected exception)\n";
    } catch(...) {
        cout << "ok " << tests_run << " - " << m << " (exception thrown)\n";
    }
}
#define mthrows(code, ...) _mthrows(string(#code), [&]() { code; }, ##__VA_ARGS__)


static void _mnothrow(string const& code, function<void()> const& f, string const& message="")
{
    ++tests_run;
    string m = string("[ ") + current_test + " ] " + ((message != "") ? message : code);
    try {
        f();
        cout << "ok " << tests_run << " - " << m << " (no exception thrown)\n";
    } catch(exception const& e) {
        cout << "not ok " << tests_run << " - " << m << " (exception thrown: " << e.what() << ")\n";
    }
}
#define mnothrow(code, ...) _mnothrow(string(#code), [&]() { code; }, ##__VA_ARGS__)

//
// MAIN PROCEDURE
//
static void prepare_tests();

int main(int argc, char* argv[])
{
    // load tests
    prepare_tests();

    // select tests
    if(argc != 1) {
        test_list.erase(remove_if(begin(test_list), end(test_list), [&argc, &argv](auto const& lst) {
            for(int i=1; i<argc; ++i) {
                if(lst.desc == string(argv[i])) {
                    return false;
                }
            }
            return true;
        }), end(test_list));
    }

    // run tests
    for(auto const& test: test_list) {
        current_test = test.desc;
        test.f();
    }
        
    cout << "1.." << tests_run << "\n";
}


// }}}

// {{{ TEST TOOL

static void test_tool()
{
    int x = 0;
    mequals(1 + 1, 2);
    mequals(1 + 1, 2, "test with message");
    mthrows(vector<int> i(1); i.at(20) = 1;, "catching exceptions");
    mnothrow(x = 1, "no exceptions");
}

// }}}

// {{{ BYTECODE GENERATOR

static void bytecode_generation()
{
    {
        Bytecode b;
        b.Add(PNIL);

        vector<uint8_t> expected = {
            0x20, 0xE2, 0x0E, 0xFF, 0x01, 0x00, 0x01, 0x00,   // magic + version
            0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // string position
            PNIL,
        };
        mequals(b.GenerateZB(), expected);
    }

    {
        Bytecode b;
        b.Add(PN8, 0x24_u8);

        vector<uint8_t> expected = {
            0x20, 0xE2, 0x0E, 0xFF, 0x01, 0x00, 0x01, 0x00,   // magic + version
            0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // string position
            PN8,  0x24,
        };
        mequals(b.GenerateZB(), expected);
    }

    {
        Bytecode b;
        b.Add(PARY, 0x1224_u16);

        vector<uint8_t> expected = {
            0x20, 0xE2, 0x0E, 0xFF, 0x01, 0x00, 0x01, 0x00,   // magic + version
            0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // string position
            PARY, 0x24, 0x12,
        };
        mequals(b.GenerateZB(), expected);
    }

    {
        Bytecode b;
        b.Add(PTBL, 0x1234_u16, 0x56_u8);

        vector<uint8_t> expected = {
            0x20, 0xE2, 0x0E, 0xFF, 0x01, 0x00, 0x01, 0x00,   // magic + version
            0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // string position
            PTBL, 0x34, 0x12, 0x56,
        };
        mequals(b.GenerateZB(), expected);
    }

    {
        Bytecode b;
        b.Add(BT, 0x12345678_u32);

        vector<uint8_t> expected = {
            0x20, 0xE2, 0x0E, 0xFF, 0x01, 0x00, 0x01, 0x00,   // magic + version
            0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // string position
            BT,   0x78, 0x56, 0x34, 0x12,
        };
        mequals(b.GenerateZB(), expected);
    }

    {
        Bytecode b;
        b.Add(PNUM, 3.1416);

        // number generated from <http://www.binaryconvert.com/convert_double.html>
        vector<uint8_t> expected = {
            0x20, 0xE2, 0x0E, 0xFF, 0x01, 0x00, 0x01, 0x00,   // magic + version
            0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // string position
            PNUM, 0xA7, 0xE8, 0x48, 0x2E, 0xFF, 0x21, 0x09, 0x40,
        };
        mequals(b.GenerateZB(), expected);
    }

    {
        Bytecode b;
        mthrows(b.Add(PNUM));
    }
}


static void bytecode_strings()
{
    Bytecode b;
    b.Add(PSTR, "hello");
    uint64_t h = hash<string>()("hello");

#pragma GCC diagnostic ignored "-Wnarrowing"
#pragma GCC diagnostic push
    vector<uint8_t> expected = {
        0x20, 0xE2, 0x0E, 0xFF, 0x01, 0x00, 0x01, 0x00,   // magic + version
        0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // string position
        PSTR, 0x00, 0x00, 0x00, 0x00,
        'h',  'e',  'l',  'l',  'o', 0,                   // string
        h & 0xFF, (h >> 8) & 0xFF, (h >> 16) & 0xFF, (h >> 24) & 0xFF, // hash
        (h >> 32) & 0xFF, (h >> 40) & 0xFF, (h >> 48) & 0xFF, (h >> 56)
    };
#pragma GCC diagnostic pop
    mequals(b.GenerateZB(), expected);
}


static void bytecode_readback()
{
    Bytecode b1;
    b1.Add(PSTR, "hello");
    vector<uint8_t> data = b1.GenerateZB();

    Bytecode b2(data);
    mequals(b2.GenerateZB(), b1.GenerateZB());

    mthrows(Bytecode(vector<uint8_t>{}));
}


static void bytecode_labels()
{
    Bytecode bc;
    Label x = bc.CreateLabel();

    for(int i=0; i<0x10; ++i) {
        bc.Add(UNM);
    }
    bc.SetLabel(x);

    for(int i=0; i<0x10; ++i) {
        bc.Add(UNM);
    }
    bc.AddLabel(x);
    for(int i=0; i<0x10; ++i) {
        bc.Add(UNM);
    }
    bc.GenerateZB();

    mequals(bc.Code()[0x20], 0x10, "Label set (byte #0)");
    mequals(bc.Code()[0x21], 0x00, "Label set (byte #1)");
}


static void bytecode_parse()
{
    Bytecode b("3");

    vector<uint8_t> expected = {
        0x20, 0xE2, 0x0E, 0xFF, 0x01, 0x00, 0x01, 0x00,   // magic + version
        0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // string position
        PN8,  0x03,
    };
    mequals(b.GenerateZB(), expected);
}

// }}}

// {{{ VIRTUAL MACHINE

static void vm_stack()
{
    ZoeVM Z;
    
    mequals(Z.StackSize(), 1);
    Z.Push(make_shared<ZNumber>(10));
    mequals(Z.StackSize(), 2);
    mequals(Z.GetPtr<ZNumber>()->Value, 10);
    mequals(Z.GetCopy<ZNumber>()->Value, 10);

    Z.Pop();
    mequals(Z.StackSize(), 1);
    Z.Pop();
    mequals(Z.StackSize(), 0);
    mthrows(Z.Pop());
}

static void vm_stack_type()
{
    ZoeVM Z;

    mequals(Z.StackSize(), 1);
    mequals(Z.GetType(), NIL);
    mthrows(Z.GetPtr<ZNumber>());
}

static void vm_stack_pnil()
{
    Bytecode b;
    b.Add(PNIL);

    ZoeVM Z; Z.ExecuteBytecode(b.GenerateZB());

    mequals(Z.StackSize(), 2);
    mequals(Z.GetType(), NIL);
}

static void vm_stack_bool()
{
    Bytecode b;
    b.Add(PBT);
    b.Add(PBF);

    ZoeVM Z; Z.ExecuteBytecode(b.GenerateZB());

    mequals(Z.StackSize(), 3);
    mequals(Z.GetType(), BOOL);
    auto v = Z.Pop<ZBool>();
    mequals(v->Value, false);
    mequals(Z.GetPtr<ZBool>()->Value, true);
}

static void vm_stack_number()
{
    Bytecode b;
    b.Add(PN8, 120_u8);
    b.Add(PNUM, 3.1416);

    ZoeVM Z; Z.ExecuteBytecode(b.GenerateZB());
    auto v = Z.Pop<ZNumber>();
    mequals(v->Value, 3.1416);
    mequals(Z.GetPtr<ZNumber>()->Value, 120);
}

static void vm_stack_string()
{
    Bytecode b;
    b.Add(PSTR, "hello");

    ZoeVM Z; Z.ExecuteBytecode(b.GenerateZB());
    mequals(Z.GetPtr<ZString>()->Value(), "hello");
}

static void vm_stack_array()
{
    Bytecode b;
    b.Add(PNUM, 3.1416);
    b.Add(PSTR, "hello");
    b.Add(PARY, 2_u16);

    ZoeVM Z; Z.ExecuteBytecode(b.GenerateZB());

    auto const& items = Z.GetPtr<ZArray>()->Items();
    mequals(items.size(), 2);
    mequals(items.at(0)->Type(), NUMBER);
    mequals(dynamic_pointer_cast<ZNumber>(items.at(0))->Value, 3.1416);
    mequals(items.at(1)->Type(), STRING);
}

static void vm_stack_table()
{
    Bytecode b;

    // { hello: 'world', [42]: 'answer', answer: 42 }
    b.Add(PSTR, "hello");
    b.Add(PSTR, "world");
    b.Add(PN8, 42_u8);
    b.Add(PSTR, "answer");
    b.Add(PSTR, "answer");
    b.Add(PN8, 42_u8);
    b.Add(PTBL, 3_u16, PUB|MUT);

    ZoeVM Z; Z.ExecuteBytecode(b.GenerateZB());

    auto const& items = Z.GetPtr<ZTable>()->Items();
    mequals(dynamic_pointer_cast<ZString>(items.at(make_shared<ZString>("hello")))->Value(), "world");
    mequals(dynamic_pointer_cast<ZString>(items.at(make_shared<ZNumber>(42)))->Value(), "answer");
    mequals(dynamic_pointer_cast<ZNumber>(items.at(make_shared<ZString>("answer")))->Value, 42);
    mthrows(items.at(make_shared<ZString>("nothing")));
}

static void vm_stack_pop()
{
    Bytecode b;
    b.Add(POP);

    ZoeVM Z; Z.ExecuteBytecode(b.GenerateZB());

    mequals(Z.StackSize(), 0);
}

// }}}

static void prepare_tests()
{
    // test tool
    run_test(test_tool);

    // bytecode
    run_test(bytecode_generation);
    run_test(bytecode_strings);
    run_test(bytecode_readback);
    run_test(bytecode_labels);
    run_test(bytecode_parse);

    // VM
    run_test(vm_stack);
    run_test(vm_stack_type);
    run_test(vm_stack_pnil);
    run_test(vm_stack_bool);
    run_test(vm_stack_number);
    run_test(vm_stack_string);
    run_test(vm_stack_array);
    run_test(vm_stack_table);
    run_test(vm_stack_pop);
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
