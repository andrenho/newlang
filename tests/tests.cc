#include <cstdlib>
#include <iostream>
#include <string>
using namespace std;

int main() 
{
    string* s = new string("abc");
    cout << "TAP version 13" << endl;
    cout << "1..3" << endl;
    cout << "ok 1 Test #1" << endl;
    cout << "ok 2 Test # Directive" << endl;
    cout << "ok 3 Test #3" << endl;
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
