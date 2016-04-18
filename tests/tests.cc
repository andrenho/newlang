#include <cstdlib>

#include <algorithm>
#include <iostream>
#include <vector>
#include <stdexcept>
using namespace std;

#include "lib/bytecode.h"
#include "lib/opcode.h"
#include "lib/zoe.h"

// {{{ TEST INFRASTRUCTURE

#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wsign-promo"

#define RED      "\033[1;31m"
#define DIMRED   "\033[2;31m"
#define GREEN    "\033[1;32m"
#define DIMGREEN "\033[2;32m"
#define BLUE     "\033[1;34m"
#define NORMAL   "\033[0m"

static int tests_run = 0;

// 
// exceptions
//
class bad_assertion : public runtime_error {
public:
    bad_assertion(const char* msg) : runtime_error(msg) {}
    bad_assertion(string const& msg) : runtime_error(msg) {}
};


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
// BASIC assertions
//
template<typename T> static void _mequals(string const& code, function<T()> const& f, T const& expected, string const& message="")
{
    ++tests_run;
    string m = (message != "") ? message : code;
    try {
        T result = f();
        if(result != expected) {
            throw bad_assertion("expected " + to_string(expected) + ", received " + to_string(result));
        }
        cout << DIMGREEN "   [ok] " NORMAL << m << " == " << to_string(expected) << endl;
    } catch(exception const& e) {
        cout << DIMRED "   [err] " NORMAL << m << endl;
        throw;
    }
}
#define mequals(code, expected, ...) _mequals<decltype(expected)>(string(#code), [&]() { return code; }, expected, ##__VA_ARGS__)


static void _mthrows(string const& code, function<void()> const& f, string const& message="")
{
    ++tests_run;
    string m = (message != "") ? message : code;
    try {
        f();
        cout << DIMRED "   [err] " NORMAL << m << endl;
        throw bad_assertion(m + ": expected exception");
    } catch(...) {
        cout << DIMGREEN "   [ok] " NORMAL << m << endl;
    }
}
#define mthrows(code, ...) _mthrows(string(#code), [&]() { code; }, ##__VA_ARGS__)


static void _mnothrow(string const& code, function<void()> const& f, string const& message="")
{
    ++tests_run;
    string m = (message != "") ? message : code;
    try {
        f();
        cout << DIMGREEN "   [ok] " NORMAL << m << endl;
    } catch(...) {
        cout << DIMRED "   [err] " NORMAL << m << endl;
        throw bad_assertion(m + ": exception thrown");
    }
}
#define mnothrow(code, ...) _mnothrow(string(#code), [&]() { code; }, ##__VA_ARGS__)


template<typename S, typename T> static void zequals(S const& code, T const& expected)
{
    Zoe Z;
    Z.Eval(string(code));
    Z.Call(0);
    T r = Z.Pop<T>();
    _mequals<T>(code, [&]() { return r; }, expected);
}

template<typename S> static void zequals(S const& code, const char* expected)
{
    zequals(code, string(expected));
}

template<typename S> static void zequals(S const& code, nullptr_t)
{
    Zoe Z;
    Z.Eval(string(code));
    Z.Call(0);
    _mequals<ZType>(code, [&]() { return Z.GetType(-1); }, NIL);
}


static void zinspect(const char* code, const char* expected)
{
    Zoe Z;
    Z.Eval(string(code));
    Z.Call(0);
    Z.Inspect(-1);
    string r = Z.Pop<string>();
    _mequals<string>(code, [&]() { return r; }, string(expected));
}


static void zthrows(const char* code)
{
    Zoe Z;
    try {
        Z.Eval(string(code));
        Z.Call(0);
        cout << DIMRED "   [err] " NORMAL << code << endl;
    } catch(...) {
        cout << DIMGREEN "   [ok] " NORMAL << code << endl;
        return;
    }
    throw bad_assertion(string(code) + ": did not throw");
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
    string current_test;
    try {
        for(auto const& test: test_list) {
            current_test = test.desc;
            cout << BLUE "{{ " << test.desc << " }}" NORMAL << endl;
            test.f();
        }
        cout << GREEN;
        cout << ".-----------------------." << endl
             << "|   ALL TESTS PASSED!   |" << endl
             << "'-----------------------'" << endl << NORMAL;
    } catch(exception const& e) {
        cout << RED "what(): " << e.what() << NORMAL << endl;
        throw;
    }
        
    cout << "Tests run: " << tests_run << endl;
}

// }}}

// {{{ TEST TOOL

static void test_tool()
{
    mequals(1 + 1, 2);
    mequals(1 + 1, 2, "test with message");
    mthrows(vector<int> i(1); i.at(20) = 1;, "catching exceptions");
    mnothrow(1, "no exceptions");
}

// }}}

// {{{ TEST BYTECODE

static vector<uint8_t> expected = {
    0x90, 0x6F, 0x65, 0x20, 0xEB, 0x00, 0x01, 0x00,     // header
    0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // code_pos
    0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // code_sz
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // data_pos
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // data_sz
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // debug_pos
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // debug_sz
    POP, PUSH_N, 0xA7, 0xE8, 0x48, 0x2E, 0xFF, 0x21, 0x09, 0x40,  // PUSH_N 3.1416
};


static void bytecode_gen() 
{
    Bytecode bc;

    bc.Add(POP);
    bc.Add(PUSH_N);
    bc.Add64<double>(3.1416);

    vector<uint8_t> found = bc.GenerateZB();
    mequals(found, expected);
}


static void bytecode_string()
{
    Bytecode bc;

    bc.AddString("ABC");
    mequals(bc.Code().size(), 4, "no NULL terminator");
    mequals(bc.Code()[0], 'A');
    mequals(bc.Code()[1], 'B');
    mequals(bc.Code()[2], 'C');
    mequals(bc.Code()[3], 0);
}


static void bytecode_import()
{
    Bytecode bc(expected);

    mequals(bc.VersionMinor(), 0x1);
    mequals(bc.Code().size(), 10);
    mequals(bc.Code()[1], static_cast<uint8_t>(PUSH_N));
    mequals(bc.Code()[2], 0xA7);
    mequals(bc.Code()[9], 0x40);

    mthrows(Bytecode b(vector<uint8_t>{ 0x00 }), "Invalid bytecode file");
}


static void bytecode_labels()
{
    Bytecode bc;
    Label x = bc.CreateLabel();

    for(int i=0; i<0x10; ++i) {
        bc.Add(0);
    }
    bc.SetLabel(x);

    for(int i=0; i<0x10; ++i) {
        bc.Add(0);
    }
    bc.AddLabel(x);
    for(int i=0; i<0x10; ++i) {
        bc.Add(0);
    }
    bc.GenerateZB();

    mequals(bc.Code()[0x20], 0x10, "Label set (byte #0)");
    mequals(bc.Code()[0x21], 0x00, "Label set (byte #1)");
}


static void bytecode_variables()
{
    Bytecode bc;

    bc.VariableAssignment("a", false);
    bc.VariableAssignment("b", false);
    bc.AddVariableRef("a");
    bc.AddVariableRef("b");

    mequals(bc.Get64<uint64_t>(0x00), 0, "Variable 'a'");
    mequals(bc.Get64<uint64_t>(0x08), 1, "Variable 'b'");

    mthrows(bc.AddVariableRef("c"));
}


static void bytecode_multivar()
{
    Bytecode bc;

    bc.MultivarCreate("a");
    bc.MultivarCreate("b");
    bc.MultivarCreate("c");

    bc.AddMultivarAssignment(false);
    bc.AddVariableRef("a");
    bc.AddVariableRef("b");
    mequals(bc.Get64<uint64_t>(0x00), 0, "Variable 'a'");
    mequals(bc.Get64<uint64_t>(0x08), 1, "Variable 'b'");

    bc.AddMultivarCounter();
    mequals(bc.Code()[0x10], 3, "Multivar counter before reset");

    bc.MultivarReset();
    bc.AddMultivarCounter();
    mequals(bc.Code()[0x11], 0, "Multivar counter after reset");
}


static void bytecode_scopes()
{
    Bytecode bc;

    bc.VariableAssignment("a", false);
    bc.AddVariableRef("a");

    bc.PushScope();
    bc.VariableAssignment("a", false);
    bc.AddVariableRef("a");
    bc.PopScope();

    bc.AddVariableRef("a");

    mequals(bc.Get64<uint64_t>(0x00), 0, "Variable 'a'");
    mequals(bc.Get64<uint64_t>(0x09), 1, "Variable 'a' (inside scope)");
    mequals(bc.Get64<uint64_t>(0x12), 0, "Variable 'a' (outside scope)");

    mthrows(bc.PopScope(), "Stack underflow");
}


static void bytecode_simplecode()
{
    Bytecode bc("3.1416");

    vector<uint8_t> found = bc.GenerateZB();
    mequals(found, expected);
}


// }}}

// {{{ ZOE STACK

static void zoe_stack() 
{
    Zoe Z;

    mequals(Z.Stack().size(), 1);

    double f = 3.24;
    Z.Push(f);

    mequals(Z.Stack().size(), 2, "stack size, 2 (after push)");
    mequals(Z.Peek<double>(), f);
    mequals(Z.Pop<double>(), f);
    mequals(Z.Stack().size(), 1, "stack size, 2 (after push/pop)");

    Z.Pop();
    mthrows(Z.Pop(), "stack underflow");
}

static void zoe_stack_invalid()
{
    Zoe Z;

    Z.Push(3.14);
    mthrows(Z.Pop<bool>());
}

static void zoe_stack_order()
{
    Zoe Z;

    Z.Push(1);
    Z.Push(2);
    Z.Push(3);
    mequals(Z.Stack().size(), 4);
    mequals(Z.Pop<double>(), 3);
    mequals(Z.Pop<double>(), 2);
    mequals(Z.Pop<double>(), 1);
    mnothrow(Z.Pop<nullptr_t>());
    mequals(Z.Stack().size(), 0);
}

static void zoe_string()
{
    Zoe Z;

    Z.Push("hello world");
    mequals(Z.Stack().size(), 2);
    mequals(Z.Peek<string>(), string("hello world"));

    string pop = Z.Pop<string>();
    mequals(pop, string("hello world"));
    mequals(Z.Stack().size(), 1);
}


// }}}

// {{{ EXECUTION

static void execution()
{
    Zoe Z;

    // load code
    Z.Eval("42");
    mequals(Z.Stack().size(), 2, "eval pushed into stack");
    mequals(Z.PeekType(), FUNCTION);

    // execute code
    Z.Call(0);
    mequals(Z.Stack().size(), 1, "result pushed into stack");
    mequals(Z.PeekType(), NUMBER);
    mequals(Z.Pop<double>(), 42);
}

static void inspect()
{
    Zoe Z;

    Z.Eval("42");
    Z.Call(0);
    Z.Inspect(-1);
    mequals(Z.Pop<string>(), string("42"));
}

// }}}

// {{{ EXPRESSIONS

static void math_expressions()
{
    zequals("2 + 3", 5);
    zequals("2 + 3", 5);
    zequals("2 * 3", 6);
    zequals("2 - 3", -1);
    zequals("3 / 2", 1.5);
    zequals("1 + 2 * 3", 7);
    zequals("(1 + 2) * 3", 9);
    zequals("3 %/ 2", 1);
    zequals("3 % 2", 1);
    zequals("-3 + 2", -1);
    zequals("2 ** 3", 8);
    zequals("0b11 & 0b10", 2);
    zequals("0b11 | 0b10", 3);
    zequals("0b11 ^ 0b10", 1);
    zequals("0b1000 >> 2", 2);
    zequals("0b1000 << 2", 32);
    zequals("(~0b1010) & 0b1111", 5);
    zequals("2 > 3", false);
    zequals("2 < 3", true);
    zequals("2 == 3", false);
    zequals("2 != 3", true);
    zequals("?3", false);
    zequals("?nil", true);
    zequals("!true", false);
    zequals("!false", true);
}


static void shortcircuit_expressions()
{
    // &&
    zequals("true && true", true);
    zequals("true && false", false);
    zequals("false && true", false);
    zequals("false && false", false);
    zequals("true && true && true", true);
    zequals("true && true && false", false);

    // ||
    zequals("true || true", true);
    zequals("true || false", true);
    zequals("false || true", true);
    zequals("false || false", false);
    zequals("true || true || true", true);
    zequals("true || true || false", true);
    zequals("false || false || false", false);

    // ternary
    zequals("2 < 3 ? 4 : 5", 4);
    zequals("2 >= 3 ? 4 : 5", 5);
}

// }}}

// {{{ STRINGS

static void strings()
{
    zequals("'abc'", "abc");
    zequals("'a\\nb'", "a\nb");
    zequals("'a\\x41b'", "aAb");
    zequals("'a\\x41b'", "aAb");
    zequals("'ab'..'cd'", "abcd");
    zequals("'ab'..'cd'..'ef'", "abcdef");
    zequals("'a\nf'", "a\nf");   // multiline string
    zequals("'a' 'b' 'cd'", "abcd");
    zequals("'ab$e'", "ab$e");
    zequals("'ab$'", "ab$");
    zequals("'ab${'cd'}ef'", "abcdef");
    zequals("'ab${'cd'}ef'\n", "abcdef");
    zequals("'ab${'cd'..'xx'}ef'", "abcdxxef");
    zequals("#'abcd'", 4);
    zequals("#''", 0);
    zequals("#'ab${'cd'..'xx'}ef'", 8);
}

static void string_subscripts()
{
    zequals("'abcd'[1]", "b");
    zequals("'abcd'[-1]", "d");
    zequals("'abcd'[1:2]", "b");
    zequals("'abcd'[1:3]", "bc");
    zequals("'abcd'[1:]", "bcd");
    zequals("'abcd'[:3]", "abc");
    zequals("'abcd'[-3:-1]", "bc");
}

// }}}

// {{{ COMMENTS

static void comments()
{
    zequals("2 + 3 /* test */", 5);
    zequals("/* test */ 2 + 3", 5);
    zequals("2 /* test */ + 3", 5);
    zequals("2 /* t\ne\nst */ + 3", 5);
    zequals("// test\n2 + 3", 5);
    zequals("2 + 3//test\n", 5);
    zequals("2 /* a /* b */ */", 2);  // nested comments
    zequals("2 /* /* / */ */", 2);  // nested comments
}

// }}}

// {{{ ARRAYS

static void arrays()
{
    zinspect("[]", "[]");
    zinspect("[2,3]", "[2, 3]");
    zinspect("[2,3,]", "[2, 3]");
    zinspect("[[1]]", "[[1]]");
    zinspect("[2, 3, []]", "[2, 3, []]");
    zinspect("[2, 3, ['abc', true, [nil]]]", "[2, 3, ['abc', true, [nil]]]");
}

static void array_equality()
{
    zequals("[]==[]", true);
    zequals("[2,3,]==[2,3]", true);
    zequals("[2,3,[4,'abc'],nil] == [ 2, 3, [ 4, 'abc' ], nil ]", true);
    zequals("[2,3,4]==[2,3]", false);
    zequals("[2,3,4]==[2,3,5]", false);
    zequals("[2,3,4]!=[2,3,5]", true);
}

static void array_lookup()
{
    zequals("[2,3,4][0]", 2);
    zequals("[2,3,4][1]", 3);
    zequals("[2,3,4][-1]", 4);
    zequals("[2,3,4][-2]", 3);
    zequals("[2,3,'hello'][2]", "hello");
    // TODO - test key error
}

static void array_slices()
{
    zinspect("[2,3,4][0:1]", "[2]");
    zinspect("[2,3,4][0:2]", "[2, 3]");
    zinspect("[2,3,4][1:3]", "[3, 4]");
    zinspect("[2,3,4][1:]", "[3, 4]");
    zinspect("[2,3,4][:2]", "[2, 3]");
    zinspect("[2,3,4][:]", "[2, 3, 4]");
    zinspect("[2,3,4][-1:]", "[4]");
    zinspect("[2,3,4][-2:-1]", "[3]");
    zinspect("[2,3,4][:-2]", "[2]");
}

static void array_operators()
{
    zequals("#[2, 3, 4]", 3);
    zequals("#[]", 0);
    zinspect("[2,3]..[4,5,6]", "[2, 3, 4, 5, 6]");
    zinspect("[2,3] * 3", "[2, 3, 2, 3, 2, 3]");
}

// }}}

// {{{ TABLES

static void table()
{
    zinspect("%{}", "%{}");
    zinspect("%{hello: 'world'}", "%{hello: 'world'}");
    zinspect("%{hello: 'world',}", "%{hello: 'world'}");
    zinspect("%{b: %{a:1}}", "%{b: %{a: 1}}");
    zinspect("%{hello: []}", "%{hello: []}");
    zinspect("%{[2]: 3, abc: %{d: 3}}", "%{abc: %{d: 3}, [2]: 3}"); 
}

static void table_access()
{
    zequals("%{[2]: 3}[2]", 3);
    zequals("%{hello: 'world', a: 42}['hello']", "world");
    zequals("%{hello: 'world', a: 42}['hello']", "world");
    zequals("%{hello: 'world', a: 42}.hello", "world");
    zequals("%{hello: 'world', a: 42}.a", 42);
    zequals("%{hello: %{world: 42}}.hello.world", 42);
    zequals("%{hello: %{world: 42}}['hello']['world']", 42);

    Zoe Z;
    Z.Eval("%{hello: 'world'}.a");
    mthrows(Z.Call(0), "Key error");
}

static void table_equality()
{
    zequals("%{}==%{}", true);
    zequals("%{hello: 'world'}==%{hello: 'world'}", true);
    zequals("%{b: %{a:1}}==%{b: %{a:1}}", true);
    zequals("%{[2]: 3, abc: %{d: %{e: 42}} }==%{[2]: 3, abc: %{d: %{e: 42} }}", true);
    zequals("%{a: 1, b: 2} == %{b: 2, a: 1}", true);
    zequals("%{}==%{hello: 'world'}", false);
    zequals("%{hello: 'world'}==%{}", false);
    zequals("%{b: %{a:1}}==%{b: 1}", false);
    zequals("%{b: %{a:1}}==%{b: %{a:2}}", false);
    zequals("%{b: %{a:1}}==%{b: %{c:1}}", false);
}

// }}}

// {{{ VARIABLES

static void variables()
{
    zequals("let a = 4", 4);
    zequals("let a = 4; a", 4);
    zequals("let a = 4; let b = 25; a", 4);
    zequals("let a = 4; let b = 25; b", 25);
    zequals("let a = let b = 25; a", 25);
    zequals("let a = let b = 25; b", 25);
    zequals("let a = 25; let b = a; b", 25);
    zequals("let a = 25; let b = a; let c = b; c", 25);
    zequals("let a = 25, b = 13; a", 25);
    zequals("let a = 25, b = 13, c = 48; b", 13);
    zequals("let a = 25, b = a, c = b; b", 25);
    zthrows("let a = 4; let a = 5; a");
    zequals("let a; a", nullptr);
}

static void multiple_assignment()
{
    zequals("let [a, b] = [3, 4]; a", 3);
    zequals("let [a, b, c] = [3, 4, 5]; b", 4);
    zequals("let x = 8, [a, b, c] = [3, x, 5]; b", 8);
    zequals("let [a, b, c] = [3, 4, 5], x = b; x", 4);
    zthrows("let [a, b] = [2, 4, 5]");
    zthrows("let [a, a] = [3, 4]");
}

static void variable_set()
{
    zequals("let a; a = 4; a", 4);
    zthrows("let a = 4; a = 5");  // changing a constant
    zthrows("let a; b = 4");      // invalid variable
    zthrows("a = 4");             // invalid variable
    
    zequals("let mut a = 4; a = 5; a", 5);
    zequals("let mut a = 4; let mut b = a; a = 5; b", 4);

    zequals("let a = true; a", true);
    zequals("let a = 'abc'; a", "abc");
    zequals("let a = [ 1, 2, 3 ]; a[1]", 2);
    zequals("let a = %{ hello: 'world', abc: 42 }; a['hello']", "world");
    zequals("let a = %{ hello: 'world', abc: 42 }; a.hello", "world");
    zequals("let a = %{ hello: %{ test: 'xxx' }, abc: 42 }; a.hello.test", "xxx");
    zequals("let a = %{ hello: 'world', abc: 42 }; a.abc", 42);
}

// }}}

// {{{ SCOPES

static void scopes()
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
}

static void scope_vars()
{
    zequals("let a = 4; { 4 }; a", 4);
    zequals("let a = 4; { let a=5; a }", 5);
    zequals("let a = 4; { let a=5; }; a", 4);
    zequals("let a = 4, b=6; { let a=5; }; b", 6);
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

static void prepare_tests()
{
    // test tool
    run_test(test_tool);

    // bytecode
    run_test(bytecode_gen);
    run_test(bytecode_string);
    run_test(bytecode_import);
    run_test(bytecode_labels);
    run_test(bytecode_variables);
    run_test(bytecode_multivar);
    run_test(bytecode_scopes);
    run_test(bytecode_simplecode);

    // ZoeVM
    run_test(zoe_stack);
    run_test(zoe_stack_invalid);
    run_test(zoe_stack_order);
    run_test(zoe_string);

    // expressions & syntax
    run_test(execution);
    run_test(inspect);
    run_test(math_expressions);
    run_test(shortcircuit_expressions);
    run_test(strings);
    run_test(string_subscripts);
    run_test(comments);

    // arrays
    run_test(arrays);
    run_test(array_equality);
    run_test(array_lookup);
    run_test(array_slices);
    run_test(array_operators);
    
    // tables
    run_test(table);
    run_test(table_access);
    run_test(table_equality);

    // variables
    run_test(variables);
    run_test(multiple_assignment);
    run_test(variable_set);
    run_test(scopes);
    run_test(scope_vars);
}


// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
