#include "src/options.h"

#include <getopt.h>
#include <iostream>
using namespace std;

#ifdef DEBUG
#include "lib/parser.h"
#include "lib/lexer.h"
#endif

Options::Options(int argc, char* argv[])
{
    mode = OperationMode::REPL;
    
    while(1) {
        static struct option long_options[] = {
#ifdef DEBUG
            { "trace",            no_argument, nullptr, 'T' },
            { "debug-bison",      no_argument, nullptr, 'B' },
            { "repl-disassemble", no_argument, nullptr, 'D' },
#endif
            { "help",             no_argument, nullptr, 'h' },
            { "version",          no_argument, nullptr, 'v' },
            { nullptr, 0, nullptr, 0 },
        };

        int opt_idx = 0;
        static const char* opts = "hv"
#ifdef DEBUG
            "TBD"
#endif
        ;
        switch(getopt_long(argc, argv, opts, long_options, &opt_idx)) {
#ifdef DEBUG
            case 'T':
                trace = true;
                break;
            case 'B':
                yydebug = 1;
                break;
            case 'D':
                repl_disassemble = true;
                break;
#endif

            case 'v':
                cout << "zoe " VERSION " - a programming language.\n";
                cout << "Avaliable under the LGPLv3 license. See COPYING file.\n";
                cout << "Written by André Wagner.\n";
                exit(EXIT_SUCCESS);
            case 'h':
                PrintHelp(cout, EXIT_SUCCESS);
                break;

            case 0:
                break;
            case -1:
                goto done;
            case '?':
                PrintHelp(cout, EXIT_FAILURE);
                break;
            default:
                abort();
        }
    }
done:
    if(optind < argc) {
        cerr << "Running scripts is still not supported.\n";
        abort();
    }

}


void Options::PrintHelp(ostream& ss, int status) const
{
    ss << "Usage: zoe [OPTION]... [SCRIPT [ARGS]...]\n";
    ss << "Avaliable options are:\n";
#ifdef DEBUG
    ss << "   -B, --debug-bison         activate BISON debugger\n";
    ss << "   -D, --repl-disassemble    disassemble when using REPL\n";
    ss << "   -T, --trace               trace assembly code execution\n";
#endif
    ss << "   -h, --help                display this help and exit\n";
    ss << "   -v, --version             show version and exit\n";
    exit(status);
}


// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=cpp
