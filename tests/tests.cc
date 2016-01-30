#include <cstdlib>
#include <iostream>
#include <vector>
using namespace std;

#include "lib/bytecode.h"
#include "lib/opcode.h"
#include "lib/zoe.h"

// {{{ TEST INFRASTRUCTURE

#pragma GCC diagnostic ignored "-Wfloat-equal"

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
            return message;                                             \
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
            return message;                                             \
        } catch(const string& e) {                                      \
            cout << DIMRED "   [err]" NORMAL " " << message << ": "     \
                 << e << endl;                                          \
            return message;                                             \
        }                                                               \
    } while (0);                                                        \
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

static const char* all_tests();

int main()
{
    try {
        return run_all_tests();
    } catch(const exception& e) {
        cout << DIMRED << "exception: " << e.what() << NORMAL << endl;
    } catch(const string& e) {
        cout << DIMRED << "exception: " << e << NORMAL << endl;
    }
    return EXIT_FAILURE;
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
    bc.AddF64(3.1416);
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

    massert(bc.GetF64(0x01) == -1, "Variable 'a'");
    massert(bc.GetF64(0x0B) == -2, "Variable 'b'");

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
    massert(bc.GetF64(0x01) == -1, "Variable 'a'");
    massert(bc.GetF64(0x0B) == -2, "Variable 'b'");

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

    massert(bc.GetF64(0x01) == -1, "Variable 'a'");
    massert(bc.GetF64(0x0C) == -2, "Variable 'a' (inside scope)");
    massert(bc.GetF64(0x17) == -1, "Variable 'a' (outside scope)");

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

// {{{ ZOE EXECUTION

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

    // Execution
    run_test(execution);

    return nullptr;
}


// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
