#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>
using namespace std;

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
            cout << "not ok " << tests_run << " - " << m << " (expected " << to_string(expected) << ", found " << to_string(result) << ")" << endl;
        } else {
            cout << "ok " << tests_run << " - " << m << " == " << to_string(expected) << endl;
        }
    } catch(exception const& e) {
        cout << "not ok " << tests_run << " - " << m << " (exception thrown: " << e.what() << ")";
    }
}
#define mequals(code, expected, ...) _mequals<decltype(expected)>(string(#code), [&]() { return code; }, expected, ##__VA_ARGS__)


static void _mthrows(string const& code, function<void()> const& f, string const& message="")
{
    ++tests_run;
    string m = string("[ ") + current_test + " ] " + ((message != "") ? message : code);
    try {
        f();
        cout << "not ok " << tests_run << " - " << m << " (expected exception)" << endl;
    } catch(...) {
        cout << "ok " << tests_run << " - " << m << " (exception thrown)" << endl;
    }
}
#define mthrows(code, ...) _mthrows(string(#code), [&]() { code; }, ##__VA_ARGS__)


static void _mnothrow(string const& code, function<void()> const& f, string const& message="")
{
    ++tests_run;
    string m = string("[ ") + current_test + " ] " + ((message != "") ? message : code);
    try {
        f();
        cout << "ok " << tests_run << " - " << m << " (no exception thrown)" << endl;
    } catch(exception const& e) {
        cout << "not ok " << tests_run << " - " << m << " (exception thrown: " << e.what() << ")";
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
        
    cout << "1.." << tests_run << endl;
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

static void prepare_tests()
{
    // test tool
    run_test(test_tool);
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
