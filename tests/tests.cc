#include <cstdlib>
#include <iostream>
#include <vector>
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

#define run_test(test) {                                 \
    cout << BLUE "{{ " << #test << " }}" NORMAL << endl; \
    do {                                                 \
        const char* message = test();                    \
        if(message) {                                    \
            return message;                              \
        }                                                \
    } while(0);                                          \
    ++tests_run;                                         \
}


#define massert(test, ...) {                                                    \
    bool _v = (test);                                                           \
    do {                                                                        \
        const char* message = #test;                                            \
        if(string(#__VA_ARGS__) != string("")) {                                \
            message = "" __VA_ARGS__;                                           \
        }                                                                       \
        string color = string("\033[2;3") + (_v ? "2" : "1") + "m";             \
        cout << "   " << color << "[" << (!_v ? "err" : "ok") << "]" NORMAL " " \
             << message << endl;                                                \
        if(!_v) {                                                               \
            return message;                                                     \
        }                                                                       \
    } while (0);                                                                \
    ++tests_run;                                                                \
}


#define mthrows(test, ...) {                                            \
    do {                                                                \
        const char* message = #test;                                    \
        if(string(#__VA_ARGS__) != string("")) {                        \
            message = "" __VA_ARGS__;                                   \
        }                                                               \
        try {                                                           \
            test();                                                     \
            cout << DIMRED "   [err]" NORMAL " " << message             \
                 << " - did not throw" << endl;                         \
        } catch(...) {                                                  \
            cout << DIMGREEN "   [ok]" NORMAL " " << message << endl;   \
        }                                                               \
    } while (0);                                                        \
    ++tests_run;                                                        \
}


#define mnothrow(test, ...) {                                           \
    do {                                                                \
        const char* message = #test;                                    \
        if(string(#__VA_ARGS__) != string("")) {                        \
            message = "" __VA_ARGS__;                                   \
        }                                                               \
        try {                                                           \
            test();                                                     \
            cout << DIMGREEN "   [ok]" NORMAL " " << message << endl;   \
        } catch(const exception& e) {                                   \
            cout << DIMRED "   [err]" NORMAL " " << message << ": "     \
                 << e.what() << endl;                                   \
            throw;                                                      \
        } catch(const string& e) {                                      \
            cout << DIMRED "   [err]" NORMAL " " << message << ": "     \
                 << e << endl;                                          \
            throw;                                                      \
        }                                                               \
    } while (0);                                                        \
}


#define zassert(code, expected) {                                            \
    do {                                                                     \
        Zoe Z;                                                               \
        try {                                                                \
            Z.Eval(code);                                                    \
            Z.Call(0);                                                       \
            decltype(expected) r = Z.Pop<decltype(expected)>();              \
            if(r == expected) {                                              \
                cout << DIMGREEN "   [ok]" NORMAL " " << code << " == "      \
                     << expected << endl;                                    \
            } else {                                                         \
                cout << DIMRED "   [err]" NORMAL " " << code << ": found "   \
                     << r << ", expected " << expected << endl;              \
                return code;                                                 \
            }                                                                \
        } catch(const exception& e) {                                        \
            cout << DIMRED "   [err]" NORMAL " " << code << ": "             \
                 << e.what() << endl;                                        \
            throw;                                                           \
        } catch(const string& e) {                                           \
            cout << DIMRED "   [err]" NORMAL " " << code << ": "             \
                 << e << endl;                                               \
            throw;                                                           \
        }                                                                    \
    } while(0);                                                              \
}

#define sassert(code, expected) zassert(code, string(expected))
        
#define zinspect(code, expected) {                                           \
    do {                                                                     \
        Zoe Z;                                                               \
        try {                                                                \
            Z.Eval(code);                                                    \
            Z.Call(0);                                                       \
            Z.Inspect(-1); \
            string r = Z.Pop<string>(); \
            if(r == expected) {                                              \
                cout << DIMGREEN "   [ok]" NORMAL " " << code << " == "      \
                     << expected << endl;                                    \
            } else {                                                         \
                cout << DIMRED "   [err]" NORMAL " " << code << ": found "   \
                     << r << ", expected " << expected << endl;              \
                return code;                                                 \
            }                                                                \
        } catch(const exception& e) {                                        \
            cout << DIMRED "   [err]" NORMAL " " << code << ": "             \
                 << e.what() << endl;                                        \
            throw;                                                           \
        } catch(const string& e) {                                           \
            cout << DIMRED "   [err]" NORMAL " " << code << ": "             \
                 << e << endl;                                               \
            throw;                                                           \
        }                                                                    \
    } while(0);                                                              \
}

static const char* all_tests();

static int run_all_tests()
{
    const char *result = all_tests();
    cout << endl;
    if(result) {
        cout << RED "error: " << result << NORMAL << endl;
    } else {
        cout << GREEN;
        cout << ".-----------------------." << endl
             << "|   ALL TESTS PASSED!   |" << endl
             << "'-----------------------'" << endl << NORMAL;
    }
    cout << "Tests run: " << tests_run << endl;

    return result != nullptr;
}


int main()
{
    try {
        return run_all_tests();
    } catch(const exception& e) {
        cout << DIMRED << "exception: " << e.what() << NORMAL << endl;
        throw;
    } catch(const string& e) {
        cout << DIMRED << "exception: " << e << NORMAL << endl;
        throw;
    }
}

// }}}

// {{{ TEST TOOL

static const char* test_tool()
{
    massert(1 + 1 == 2);
    massert(1 + 1 != 3);
    massert(1 + 1 == 2, "test with message");
    mthrows([](){ vector<int> i(1); i.at(20) = 1; }, "catching exceptions");
    mnothrow([](){}, "no exceptions");
    return nullptr;
}

// }}}

// {{{ TEST BYTECODE

static vector<uint8_t> expected = {
    0x90, 0x6F, 0x65, 0x20, 0xEB, 0x00, 0x01, 0x00,     // header
    0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // code_pos
    0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // code_sz
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // data_pos
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // data_sz
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // debug_pos
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // debug_sz
    POP, PUSH_N, 0xA7, 0xE8, 0x48, 0x2E, 0xFF, 0x21, 0x09, 0x40,  // PUSH_N 3.1416
    END,                                                // EOF
};


static const char* bytecode_gen() 
{
    Bytecode bc;

    bc.Add(POP);
    bc.Add(PUSH_N);
    bc.Add64<double>(3.1416);
    bc.Add(END);

    vector<uint8_t> found = bc.GenerateZB();
    massert(found == expected);

    return nullptr;
}


static const char* bytecode_string()
{
    Bytecode bc;

    bc.AddString("ABC");
    massert(bc.Code().size() == 4, "no NULL terminator");
    massert(bc.Code()[0] == 'A');
    massert(bc.Code()[1] == 'B');
    massert(bc.Code()[2] == 'C');
    massert(bc.Code()[3] == 0);

    return nullptr;
}


static const char* bytecode_import()
{
    Bytecode bc(expected);

    massert(bc.VersionMinor() == 0x1);
    massert(bc.Code().size() == 11);
    massert(bc.Code()[1] == PUSH_N);
    massert(bc.Code()[2] == 0xA7);
    massert(bc.Code()[9] == 0x40);

    mthrows([](){ Bytecode b(vector<uint8_t>{ 0x00 }); }, "Invalid bytecode file");

    return nullptr;
}


static const char* bytecode_labels()
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

    massert(bc.Code()[0x20] == 0x10, "Label set (byte #0)");
    massert(bc.Code()[0x21] == 0x00, "Label set (byte #1)");

    return nullptr;
}


static const char* bytecode_variables()
{
    Bytecode bc;

    bc.VariableAssignment("a", false);
    bc.VariableAssignment("b", false);
    bc.AddVariable("a");
    bc.AddVariable("b");

    massert(bc.Get64<int64_t>(0x01) == -1, "Variable 'a'");
    massert(bc.Get64<int64_t>(0x0B) == -2, "Variable 'b'");

    mthrows([&](){ bc.AddVariable("c"); });

    return nullptr;
}


static const char* bytecode_multivar()
{
    Bytecode bc;

    bc.MultivarCreate("a");
    bc.MultivarCreate("b");
    bc.MultivarCreate("c");

    bc.AddMultivarAssignment(false);
    bc.AddVariable("a");
    bc.AddVariable("b");
    massert(bc.Get64<int64_t>(0x01) == -1, "Variable 'a'");
    massert(bc.Get64<int64_t>(0x0B) == -2, "Variable 'b'");

    bc.AddMultivarCounter();
    massert(bc.Code()[0x14] == 3, "Multivar counter before reset");

    bc.MultivarReset();
    bc.AddMultivarCounter();
    massert(bc.Code()[0x15] == 0, "Multivar counter after reset");

    return nullptr;
}


static const char* bytecode_scopes()
{
    Bytecode bc;

    bc.VariableAssignment("a", false);
    bc.AddVariable("a");

    bc.PushScope();
    bc.VariableAssignment("a", false);
    bc.AddVariable("a");
    bc.PopScope();

    bc.AddVariable("a");

    massert(bc.Get64<int64_t>(0x01) == -1, "Variable 'a'");
    massert(bc.Get64<int64_t>(0x0C) == -2, "Variable 'a' (inside scope)");
    massert(bc.Get64<int64_t>(0x17) == -1, "Variable 'a' (outside scope)");

    mthrows([&](){ bc.PopScope(); }, "Stack underflow");
    
    return nullptr;
}


static const char* bytecode_simplecode()
{
    Bytecode bc("3.1416");

    vector<uint8_t> found = bc.GenerateZB();
    massert(found == expected);

    return nullptr;
}


// }}}

// {{{ ZOE STACK

static const char* zoe_stack() 
{
    Zoe Z;

    massert(Z.Stack().size() == 1);

    double f = 3.24;
    Z.Push(f);

    massert(Z.Stack().size() == 2, "stack size == 2 (after push)");
    massert(Z.Peek<double>() == f);
    massert(Z.Pop<double>() == f);
    massert(Z.Stack().size() == 1, "stack size == 2 (after push/pop)");

    Z.Pop();
    mthrows([&](){ Z.Pop(); }, "stack underflow");

    return nullptr;
}

static const char* zoe_stack_invalid()
{
    Zoe Z;

    Z.Push(3.14);
    mthrows([&]() { Z.Pop<bool>(); });

    return nullptr;
}

static const char* zoe_stack_order()
{
    Zoe Z;

    Z.Push(1);
    Z.Push(2);
    Z.Push(3);
    massert(Z.Stack().size() == 4);
    massert(Z.Pop<double>() == 3);
    massert(Z.Pop<double>() == 2);
    massert(Z.Pop<double>() == 1);
    mnothrow([&]() { Z.Pop<nullptr_t>(); });
    massert(Z.Stack().size() == 0);

    return nullptr;
}

static const char* zoe_string()
{
    Zoe Z;

    Z.Push("hello world");
    massert(Z.Stack().size() == 2);
    massert(Z.Peek<string>() == "hello world");

    string pop = Z.Pop<string>();
    massert(pop == "hello world");
    massert(Z.Stack().size() == 1);

    return nullptr;
}


// }}}

// {{{ EXECUTION

static const char* execution()
{
    Zoe Z;

    // load code
    Z.Eval("42");
    massert(Z.Stack().size() == 2, "eval pushed into stack");
    massert(Z.PeekType() == FUNCTION);

    // execute code
    Z.Call(0);
    massert(Z.Stack().size(), "result pushed into stack");
    massert(Z.PeekType() == NUMBER);
    massert(Z.Pop<double>() == 42);

    return nullptr;
}

static const char* inspect()
{
    Zoe Z;

    Z.Eval("42");
    Z.Call(0);
    Z.Inspect(-1);
    massert(Z.Pop<string>() == "42");

    return nullptr;
}

// }}}

// {{{ EXPRESSIONS

static const char* math_expressions()
{
    zassert("2 + 3", 5);
    zassert("2 + 3", 5);
    zassert("2 * 3", 6);
    zassert("2 - 3", -1);
    zassert("3 / 2", 1.5);
    zassert("1 + 2 * 3", 7);
    zassert("(1 + 2) * 3", 9);
    zassert("3 %/ 2", 1);
    zassert("3 % 2", 1);
    zassert("-3 + 2", -1);
    zassert("2 ** 3", 8);
    zassert("0b11 & 0b10", 2);
    zassert("0b11 | 0b10", 3);
    zassert("0b11 ^ 0b10", 1);
    zassert("0b1000 >> 2", 2);
    zassert("0b1000 << 2", 32);
    zassert("(~0b1010) & 0b1111", 5);
    zassert("2 > 3", false);
    zassert("2 < 3", true);
    zassert("2 == 3", false);
    zassert("2 != 3", true);
    zassert("?3", false);
    zassert("?nil", true);
    zassert("!true", false);
    zassert("!false", true);

    return nullptr;
}


static const char* shortcircuit_expressions()
{
    // &&
    zassert("true && true", true);
    zassert("true && false", false);
    zassert("false && true", false);
    zassert("false && false", false);
    zassert("true && true && true", true);
    zassert("true && true && false", false);

    // ||
    zassert("true || true", true);
    zassert("true || false", true);
    zassert("false || true", true);
    zassert("false || false", false);
    zassert("true || true || true", true);
    zassert("true || true || false", true);
    zassert("false || false || false", false);

    // ternary
    zassert("2 < 3 ? 4 : 5", 4);
    zassert("2 >= 3 ? 4 : 5", 5);

    return nullptr;
}

// }}}

// {{{ STRINGS

static const char* strings()
{
    sassert("'abc'", "abc");
    sassert("'a\\nb'", "a\nb");
    sassert("'a\\x41b'", "aAb");
    sassert("'a\\x41b'", "aAb");
    sassert("'ab'..'cd'", "abcd");
    sassert("'ab'..'cd'..'ef'", "abcdef");
    sassert("'a\nf'", "a\nf");   // multiline string
    sassert("'a' 'b' 'cd'", "abcd");
    sassert("'ab$e'", "ab$e");
    sassert("'ab$'", "ab$");
    sassert("'ab${'cd'}ef'", "abcdef");
    sassert("'ab${'cd'}ef'\n", "abcdef");
    sassert("'ab${'cd'..'xx'}ef'", "abcdxxef");
    zassert("#'abcd'", 4);
    zassert("#''", 0);
    zassert("#'ab${'cd'..'xx'}ef'", 8);

    return nullptr;
}

static const char* string_subscripts()
{
    sassert("'abcd'[1]", "b");
    sassert("'abcd'[-1]", "d");
    sassert("'abcd'[1:2]", "b");
    sassert("'abcd'[1:3]", "bc");
    sassert("'abcd'[1:]", "bcd");
    sassert("'abcd'[:3]", "abc");
    sassert("'abcd'[-3:-1]", "bc");

    return nullptr;
}

// }}}

// {{{ COMMENTS

static const char* comments()
{
    zassert("2 + 3 /* test */", 5);
    zassert("/* test */ 2 + 3", 5);
    zassert("2 /* test */ + 3", 5);
    zassert("2 /* t\ne\nst */ + 3", 5);
    zassert("// test\n2 + 3", 5);
    zassert("2 + 3//test\n", 5);
    zassert("2 /* a /* b */ */", 2);  // nested comments
    zassert("2 /* /* / */ */", 2);  // nested comments
    return nullptr;
}

// }}}

// {{{ ARRAYS

static const char* arrays()
{
    zinspect("[]", "[]");
    zinspect("[2,3]", "[2, 3]");
    zinspect("[2,3,]", "[2, 3]");
    zinspect("[[1]]", "[[1]]");
    zinspect("[2, 3, []]", "[2, 3, []]");
    zinspect("[2, 3, ['abc', true, [nil]]]", "[2, 3, ['abc', true, [nil]]]");

    return nullptr;
}

static const char* array_equality()
{
    zassert("[]==[]", true);
    zassert("[2,3,]==[2,3]", true);
    zassert("[2,3,[4,'abc'],nil] == [ 2, 3, [ 4, 'abc' ], nil ]", true);
    zassert("[2,3,4]==[2,3]", false);
    zassert("[2,3,4]==[2,3,5]", false);
    zassert("[2,3,4]!=[2,3,5]", true);

    return nullptr;
}

static const char* array_lookup()
{
    zassert("[2,3,4][0]", 2);
    zassert("[2,3,4][1]", 3);
    zassert("[2,3,4][-1]", 4);
    zassert("[2,3,4][-2]", 3);
    sassert("[2,3,'hello'][2]", "hello");

    // TODO - test key error

    return nullptr;
}

static const char* array_slices()
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

    return nullptr;
}

static const char* array_operators()
{
    zassert("#[2, 3, 4]", 3);
    zassert("#[]", 0);
    zinspect("[2,3]..[4,5,6]", "[2, 3, 4, 5, 6]");
    zinspect("[2,3] * 3", "[2, 3, 2, 3, 2, 3]");

    return nullptr;
}

// }}}

// {{{ TABLES

static const char* table()
{
    zinspect("%{}", "%{}");
    zinspect("%{hello: 'world'}", "%{hello: 'world'}");
    zinspect("%{hello: 'world',}", "%{hello: 'world'}");
    zinspect("%{b: %{a:1}}", "%{b: %{a: 1}}");
    zinspect("%{hello: []}", "%{hello: []}");
    zinspect("%{[2]: 3, abc: %{d: 3}}", "%{abc: %{d: 3}, [2]: 3}"); 

    return nullptr;
}

static const char* table_access(void)
{
    zassert("%{[2]: 3}[2]", 3);
    /*
    sassert("%{hello: 'world', a: 42}['hello']", "world");
    mu_assert_sexpr("%{hello: 'world', a: 42}['hello']", "world");
    mu_assert_sexpr("%{hello: 'world', a: 42}.hello", "world");
    mu_assert_nexpr("%{hello: 'world', a: 42}.a", 42);
    mu_assert_nexpr("%{hello: %{world: 42}}.hello.world", 42);
    mu_assert_nexpr("%{hello: %{world: 42}}['hello']['world']", 42);

    Zoe* Z = zoe_createvm(test_error);
    error_found = false;
    zoe_eval(Z, "%{hello: 'world'}.a");
    zoe_call(Z, 0);
    mu_assert("key error", error_found);
    zoe_free(Z);
    */

    return nullptr;
}

// }}}

static const char* all_tests()
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

    return nullptr;
}


// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
