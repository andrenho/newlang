#include "options.h"

#include <getopt.h>
#include <cstdio>
#include <cstdlib>

Options::Options(int argc, char* argv[])
    : operation_mode(OperationMode::REPL),
      repl_options({ true })
{
    (void) argc;
    (void) argv;

    static int disassemble = 0;

    while(1) {
        static struct option long_options[] = {
            { "repl-disassemble", no_argument, &disassemble, 1 },
            { "help",     no_argument,  nullptr, 'h' },
            { "version",  no_argument,  nullptr, 'v' },
            { nullptr, 0, nullptr, 0 },
        };

        int opt_idx = 0;
        switch(getopt_long(argc, argv, "hv", long_options, &opt_idx)) {
            case 'v':
                printf("zoe " VERSION " - a programming language.\n");
                printf("Avaliable under the GPLv3 license.\n");
                printf("Written by André Wagner.\n");
                exit(EXIT_SUCCESS);
            case 'h':
                PrintHelp(stdout, EXIT_SUCCESS);
                break;

            case 0:
                break;
            case -1:
                goto done;
            case '?':
                PrintHelp(stderr, EXIT_FAILURE);
                break;
            default:
                abort();
        }
    }
done:
    repl_options.disassemble = disassemble;

    if(optind < argc) {
        fprintf(stderr, "Running script is still not supported.\n");
        abort();
    }
}


void Options::PrintHelp(FILE* f, int exit_status) const
{
    fprintf(f, "Usage: zoe [OPTION]... [SCRIPT [ARGS]...]\n");
    fprintf(f, "Avaliable options are:\n");
    fprintf(f, "   -D, --repl-disassemble    disassemble when using REPL\n");
    fprintf(f, "   -h, --help                display this help and exit\n");
    fprintf(f, "   -v, --version             show version and exit\n");
    exit(exit_status);
}


// vim: ts=4:sw=4:sts=4:expandtab
