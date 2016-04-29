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
    try {       // NOLINT - bug in linter
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
    try {       // NOLINT - bug in linter
        f();
        cout << "ok " << tests_run << " - " << m << " (no exception thrown)\n";
    } catch(exception const& e) {
        cout << "not ok " << tests_run << " - " << m << " (exception thrown: " << e.what() << ")\n";
    }
}
#define mnothrow(code, ...) _mnothrow(string(#code), [&]() { code; }, ##__VA_ARGS__)

// 
// ZoeVM execution assertions
//

static void minvalid_syntax(const char* code)
{
    ++tests_run;
    try {
        Bytecode b(code);
        cout << "not ok " << tests_run << " - " << code << " (syntax not invalid)\n";
    } catch(zoe_syntax_error const& e) {
        cout << "ok " << tests_run << " - " << code << " (syntax invalid: " << e.what() << ")\n";
    }
}


template<typename S, typename T> static void zequals(S const& code, T const& expected)
{
    try {
        ZoeVM Z;
        Bytecode b(code);
        Z.ExecuteBytecode(b.GenerateZB());
        if(Z.StackSize() != 1) {
            cout << "not ok " << tests_run << " - " << code << " (stack size = " << Z.StackSize() << ")\n";
            return;
        }
        _mequals<T>(string(code), [&]() { return Z.CopyCppValue<T>(); }, expected);
    } catch(exception const& e) {
        ++tests_run;
        cout << "not ok " << tests_run << " - " << code << " (exception thrown: " << e.what() << ")\n";
    }
}
template<typename S> static void zequals(S const& code, const char* expected)
{
    zequals(code, string(expected));
}
template<typename S> static void zequals(S const& code, nullptr_t)
{
    try {
        ZoeVM Z;
        Bytecode b(code);
        Z.ExecuteBytecode(b.GenerateZB());
        _mequals<ZType>(string(code), [&]() { return Z.GetType(-1); }, NIL);
    } catch(exception const& e) {
        ++tests_run;
        cout << "not ok " << tests_run << " - " << code << " (exception thrown: " << e.what() << ")\n";
    }
}


static void zthrows(const char* code)
{
    try {               // NOLINT - bug in linter
        ++tests_run;
        ZoeVM Z;
        Bytecode b(code);
        Z.ExecuteBytecode(b.GenerateZB());
        cout << "not ok " << tests_run << " - " << code << " (zoe exception not thrown)\n";
    } catch(zoe_runtime_error const& e) {
        cout << "ok " << tests_run << " - " << code << " (zoe exception thrown: " << e.what() << ")\n";
    } catch(zoe_syntax_error const& e) {
        cout << "ok " << tests_run << " - " << code << " (zoe exception thrown: " << e.what() << ")\n";
    } catch(exception const& e) {
        cout << "not ok " << tests_run << " - " << code << " (non-zoe exception: " << e.what() << ")\n";
    }
}


static void zinspect(const char* code, const char* expected)
{
    try {
        ZoeVM Z;
        Bytecode b(code);
        Z.ExecuteBytecode(b.GenerateZB());
        if(Z.StackSize() != 1) {
            cout << "not ok " << tests_run << " - " << code << " (stack size = " << Z.StackSize() << ")\n";
            return;
        }
        _mequals<string>(string(code), [&]() { return Z.GetPtr(-1)->Inspect(); }, expected);
    } catch(exception const& e) {
        ++tests_run;
        cout << "not ok " << tests_run << " - " << code << " (exception thrown: " << e.what() << ")\n";
    }
}


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
        0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // string position
        POP,  PN8,  0x03,
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
    mequals(Z.GetPtr<ZNumber>()->Value(), 10);
    mequals(Z.GetCopy<ZNumber>()->Value(), 10);

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
    mequals(v->Value(), false);
    mequals(Z.GetPtr<ZBool>()->Value(), true);
}

static void vm_stack_number()
{
    Bytecode b;
    b.Add(PN8, 120_u8);
    b.Add(PNUM, 3.1416);

    ZoeVM Z; Z.ExecuteBytecode(b.GenerateZB());
    auto v = Z.Pop<ZNumber>();
    mequals(v->Value(), 3.1416);
    mequals(Z.GetPtr<ZNumber>()->Value(), 120);
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

    auto const& items = Z.GetPtr<ZArray>()->Value();
    mequals(items.size(), 2);
    mequals(items.at(0)->Type(), NUMBER);
    mequals(dynamic_pointer_cast<ZNumber>(items.at(0))->Value(), 3.1416);
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
    b.Add(PTBX, 3_u16);

    ZoeVM Z; Z.ExecuteBytecode(b.GenerateZB());

    auto const& items = Z.GetPtr<ZTable>()->Value();
    mequals(dynamic_pointer_cast<ZString>(items.at(make_shared<ZString>("hello")).value)->Value(), "world");
    mequals(dynamic_pointer_cast<ZString>(items.at(make_shared<ZNumber>(42)).value)->Value(), "answer");
    mequals(dynamic_pointer_cast<ZNumber>(items.at(make_shared<ZString>("answer")).value)->Value(), 42);
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

// {{{ ZOE BASIC EXECUTION

static void zoe_invalid()
{
    minvalid_syntax("3 3");
}

static void zoe_literals()
{
    zequals("3.14", 3.14);
    zequals("0xABCD_EF01", 0xABCDEF01);
    zequals("0b10_1010", 0b101010);
    zequals("0o1234", 01234);
    zequals("true", true);
    zequals("false", false);
    zequals("nil", nullptr);
    zequals("'hello'", "hello");
    zequals("'he' 'llo'", "hello");
    zequals("'he' 'l' 'lo'", "hello");
    zequals("'he${'ll'}o'", "hello");
    zequals("'${'hell'}o'", "hello");
    zequals("'he${'llo'}'", "hello");
}

static void zoe_strings()
{
    zequals("'abc'", "abc");
    zequals("'a\\nb'", "a\nb");
    zequals("'a\\x41b'", "aAb");
    zequals("'a\\x41b'", "aAb");
    zequals("'a\nf'", "a\nf");   // multiline string
    zequals("'a' 'b' 'cd'", "abcd");
    zequals("'ab$e'", "ab$e");
    zequals("'ab$'", "ab$");
    zequals("'ab${'cd'}ef'", "abcdef");
    zequals("'ab${'cd'}ef'\n", "abcdef");
    zequals("'ab${'cd' 'xx'}ef'", "abcdxxef");
}

static void zoe_inspection()
{
    zinspect("3", "3");
    zinspect("3.14", "3.14");
    zinspect("true", "true");
    zinspect("'hello'", "'hello'");
}

// }}}

// {{{ COMMENTS

static void comments()
{
    zequals("'a' 'b' /* test */", "ab");
    zequals("/* test */ 'a' 'b'", "ab");
    zequals("'a' /* test */ 'b'", "ab");
    zequals("'a' /* t\ne\nst */ 'b'", "ab");
    zequals("// test\n'a' 'b'", "ab");
    zequals("'a' 'b'//test\n", "ab");
    zequals("'a' /* a /* b */ */ 'b'", "ab");  // nested comments
    zequals("'a' /* /* / */ */ 'b'", "ab");  // nested comments
}

// }}}

// {{{ ZOE VARIABLES 

static void zoe_variables()
{
    zequals("let a = 4", 4);
    zequals("let a = 4; a", 4);
    zequals("let a = 4; let b = 25; a", 4);
    zequals("let a = 4; let b = 25; b", 25);
    zequals("let a = let b = 25; a", 25);
    zequals("let a = let b = 25; b", 25);
    zequals("let a = 25; let b = a; b", 25);
    zequals("let a = 25; let b = a; let c = b; c", 25);
    zthrows("let a = 4; let a = 5; a");
    zequals("let a; a", nullptr);
}

static void zoe_multivariables()
{
    zequals("let [a, b] = [3, 4]; a", 3);
    zequals("let [a, b, c] = [3, 4, 5]; b", 4);
    zthrows("let [a, b] = [2, 4, 5]");
    zthrows("let [a, a] = [3, 4]");
}

static void zoe_variable_set()
{
    zequals("let mut a; a = 4; a", 4);
    zthrows("let a = 4; a = 5");  // changing a constant
    zthrows("let a; b = 4");      // invalid variable
    zthrows("a = 4");             // invalid variable
    
    zequals("let mut a = 4; a = 5; a", 5);
    zequals("let mut a = 4; let mut b = a; a = 5; b", 4);

    zequals("let a = true; a", true);
    zequals("let a = 'abc'; a", "abc");

    zequals("let a = &{ hello: 'world', abc: 42 }; a['hello']", "world");
    zequals("let a = &{ hello: 'world', abc: 42 }; a.hello", "world");
    zequals("let a = &{ hello: &{ test: 'xxx' }, abc: 42 }; a.hello.test", "xxx");
    zequals("let a = &{ hello: 'world', abc: 42 }; a.abc", 42);
}

static void zoe_scopes()
{
    zequals("{ 4 }", 4);
    zequals("{ 4; 5 }", 5);
    zequals("{ 4; 5; }", 5);
    zequals("{ 4\n 5 }", 5);
    zequals("{ 4; { 5; } }", 5);
    zequals("{ 4; { 5; }; }", 5);
    zequals("{ 4; { 5; { 6; 7 } } }", 7);
    zequals("{ 4; { 5; { 6; 7 } } }", 7);
    zequals("{ 4; { 5; { 6; 7 } } }", 7);
    zequals("{ 4 }; 5", 5);
    zequals("{ 4\n { 5; { 6; 7; } } }; 8", 8);
    zequals("{ \n 4 \n }\n 5", 5);

    zequals("let a = 42; { let a = 12 }; a", 42);
    zequals("let a = 4; { 4 }; a", 4);
    zequals("let a = 4; { let a=5; a }", 5);
    zequals("let a = 4; { let a=5; }; a", 4);
    zequals("let [a, b] = [4, 6]; { let a=5; }; b", 6);
    zequals("let a = 4; { let a=5; { let a=6 }; a }", 5);
    zequals("let a = 4; { let a=5; { let a=6 } }; a", 4);
    zequals("let a = 4; { let a=5 } ; { let a=6 }; a", 4);
    zequals("let a = 4; { let a=5 } ; { let a=6; a }", 6);
    zequals("let a = 4; { let a=5 } ; { let b=6; a }", 4);
    zthrows("{ let a=4 }; a");

    zequals("let mut a = 4; { let a=5 } ; { let b=6; a }", 4);
    zequals("let mut a = 4; { a=5 } ; { let b=6; a }", 5);

    zequals("let mut a = 4; { let a = 5 }; a", 4);
    zequals("let mut a = 4; { let a = 5; a }", 5);
}

// }}}

// {{{ ARRAYS & TABLES

static void zoe_array_init()
{
    zinspect("[]", "[]");
    zinspect("[2,3]", "[2, 3]");
    zinspect("[2,3,]", "[2, 3]");
    zinspect("[[1]]", "[[1]]");
    zinspect("[2, 3, []]", "[2, 3, []]");
    zinspect("[2, [3, 4], 5]", "[2, [3, 4], 5]");
    zinspect("[2, 3, ['abc', true, [nil]]]", "[2, 3, ['abc', true, [nil]]]");
}

static void zoe_table_init()
{
    zinspect("%{}", "%{}");
    zinspect("%{hello: 'world'}", "%{hello: 'world'}");
    zinspect("%{hello: 'world',}", "%{hello: 'world'}");
    zinspect("%{b: %{a:1}}", "%{b: %{a: 1}}");
    zinspect("%{hello: []}", "%{hello: []}");
    zinspect("%{[2]: 3, abc: %{d: 3}}", "%{abc: %{d: 3}, [2]: 3}"); 
    zinspect("&{hello: 'world'}", "&{hello: 'world'}");
}

static void zoe_table_get_set()
{
    zinspect("let a = &{}; a", "&{}");
    zequals("let mut a = &{}; a.hello = 42", 42);
    zinspect("let a = &{hello: 42}; a", "&{hello: 42}");
    zequals("let a = &{hello: 42}; a.hello", 42);
    zequals("let mut a = &{}; a['hello'] = 42; a['hello']", 42);
    zinspect("let mut a = &{}; a.b = 42; a", "&{b: 42}");
    zequals("let mut a = &{}; a.b = 42; a.b", 42);

    zequals("&{[2]: 3}[2]", 3);
    zequals("&{hello: 'world', a: 42}['hello']", "world");
    zequals("&{hello: 'world', a: 42}['hello']", "world");
    zequals("&{hello: 'world', a: 42}.hello", "world");
    zequals("&{hello: 'world', a: 42}.a", 42);
    zequals("&{hello: &{world: 42}}.hello.world", 42);
    zequals("&{hello: &{world: 42}}['hello']['world']", 42);
}

static void zoe_table_pub_mut()
{
    zequals("let mut x = &{a: 42}; x.a", 42);
    zequals("let mut x = &{a: 42}; x.a = 12; x.a", 12);
    zthrows("let mut x = %{a: 42}; x.a = 12");
    zthrows("let mut x = %{pub a: 42}; x.a = 12; x.a");   // public but not mutable
    zthrows("let mut x = %{mut a: 42}; x.a = 12; x.a");   // mutable but not public
    zthrows("let mut x = %{a: 42}; x.a");
    zequals("let mut x = %{pub a: 42}; x.a", 42);
    zequals("let mut x = %{pub mut a: 42}; x.a = 12; x.a", 12);

    zequals("let mut a = &{ hello: 'world' }; a['hello'] = 42; a['hello']", 42);
    zequals("let mut a = &{ hello: 'world' }; a.hello = 42; a.hello", 42);
    zequals("let mut a = &{ hello: 'world' }; a.test = 42; a.hello", "world");
    zequals("let mut a = &{ hello: 'world' }; a.test = 42; a.test", 42);
}

static void zoe_table_proto()
{
    zequals("let x = &{a: 42}; let b = &[x]{}; b.a", 42);
    zequals("let x = &{a: 42}; let b = &[x]{}; b.a = 12; b.a", 12);
    zequals("let x = &{a: 42}; let b = &[x]{}; x.a = 12; b.a", 12);
    zequals("let x = &{a: 42}; let b = &[x]{}; x.a = 12; x.a", 12);
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

    // execution
    run_test(zoe_invalid);
    run_test(zoe_literals);
    run_test(zoe_strings);
    run_test(zoe_inspection);

    // comments
    run_test(comments);

    // variables
    run_test(zoe_variables);
    run_test(zoe_multivariables);
    run_test(zoe_variable_set);
    run_test(zoe_scopes);

    // arrays & tables
    run_test(zoe_array_init);
    run_test(zoe_table_init);
    run_test(zoe_table_get_set);
    run_test(zoe_table_pub_mut);
    run_test(zoe_table_proto);
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
