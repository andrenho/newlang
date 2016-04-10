#include <iostream>
#include <vector>
using namespace std;

#include "lib/bytecode.h"
#include "lib/opcode.h"

// {{{ TEST INFRASTRUCTURE

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
        cout << "   " << color << "[" << (!_v ? "err" : "ok") << "]" NORMAL " "  \
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
            cout << DIMRED "   [ok]" NORMAL " " << message << endl;      \
            return message;                                             \
        } catch(const exception& e) {                                   \
            cout << DIMGREEN "   [ok]" NORMAL " " << message << endl;    \
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
            cout << DIMGREEN "   [ok]" NORMAL " " << message << endl;    \
        } catch(const exception& e) {                                   \
            cout << DIMRED "   [ok]" NORMAL " " << message << ": "       \
                 << e.what() << endl;                                   \
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

static uint8_t expected[] = {
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


static const char* bytecode_gen(void) 
{
    Bytecode bc;

    bc.Add(POP);
    bc.Add(PUSH_N);
    bc.AddF64(3.1416);
    bc.Add(END);

    vector<uint8_t> found = bc.GenerateZB();

    massert(found.size() == sizeof expected);
    size_t i = 0;
    for(auto const& d: found) {
        massert(expected[i++] == d);
    }
    
    return nullptr;
}


static const char* bytecode_import(void)
{
    Bytecode bc(expected, sizeof expected);

    massert(bc.VersionMinor() == 0x1);
    massert(bc.Code().size() == 11);
    massert(bc.Code()[1] == PUSH_N);
    massert(bc.Code()[2] == 0xA7);
    massert(bc.Code()[9] == 0x40);

    return nullptr;
}


static const char* bytecode_simplecode(void)
{
    Bytecode bc("3.1416");

    vector<uint8_t> found = bc.GenerateZB();

    massert(found.size() == sizeof expected);
    size_t i = 0;
    for(auto const& d: found) {
        massert(expected[i++] == d);
    }

    return nullptr;
}


// }}}

static const char* all_tests()
{
    // test tool
    run_test(test_tool);

    // bytecode
    run_test(bytecode_gen);
    run_test(bytecode_import);
    run_test(bytecode_simplecode);

    return nullptr;
}


int main()
{
    return run_all_tests();
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
