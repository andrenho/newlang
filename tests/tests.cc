#include <cstdlib>
#include <iostream>
using namespace std;

int main() 
{
    cout << "TAP version 13" << endl;
    cout << "1..3" << endl;
    cout << "ok 1 Test #1" << endl;
    cout << "not ok 2 Test # Directive" << endl;
    cout << "# Diagnostic" << endl;
    cout << "  ---" << endl;
    cout << "  message: 'Failure message'" << endl;
    cout << "  severity: fail" << endl;
    cout << "    data:" << endl;
    cout << "      got:" << endl;
    cout << "        - 1" << endl;
    cout << "      expected:" << endl;
    cout << "        - 2" << endl;
    
    cout << "ok 3 Test #3" << endl;
    abort();
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
