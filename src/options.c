#include "src/options.h"

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef DEBUG
#include "lib/parser.tab.h"
#include "lib/lex.yy.h"
#endif

static void options_printhelp(FILE* f, int exit_status);

Options*
options_parse(int argc, char* argv[])
{
    Options* opt = calloc(sizeof(Options), 1);

    while(1) {
        static struct option long_options[] = {
#ifdef DEBUG
            { "debug-asm",        no_argument, NULL, 'A' },
            { "debug-bison",      no_argument, NULL, 'B' },
            { "repl-disassemble", no_argument, NULL, 'D' },
#endif
            { "help",             no_argument, NULL, 'h' },
            { "version",          no_argument, NULL, 'v' },
            { NULL, 0, NULL, 0 },
        };

        int opt_idx = 0;
        static const char* opts = "hv"
#ifdef DEBUG
            "ABD"
#endif
        ;
        switch(getopt_long(argc, argv, opts, long_options, &opt_idx)) {
#ifdef DEBUG
            case 'A':
                opt->debug_asm = true;
                break;
            case 'B':
                yydebug = 1;
                break;
            case 'D':
                opt->repl_options.disassemble = true;
                break;
#endif

            case 'v':
                printf("zoe " VERSION " - a programming language.\n");
                printf("Avaliable under the LGPLv3 license. See COPYING file.\n");
                printf("Written by André Wagner.\n");
                exit(EXIT_SUCCESS);
            case 'h':
                options_printhelp(stdout, EXIT_SUCCESS);
                break;

            case 0:
                break;
            case -1:
                goto done;
            case '?':
                options_printhelp(stderr, EXIT_FAILURE);
                break;
            default:
                abort();
        }
    }
done:
    if(optind < argc) {
        fprintf(stderr, "Running scripts is still not supported.\n");
        abort();
    }

    return opt;
}


void
options_free(Options* opt)
{
    free(opt);
}


static void
options_printhelp(FILE* f, int exit_status)
{
    fprintf(f, "Usage: zoe [OPTION]... [SCRIPT [ARGS]...]\n");
    fprintf(f, "Avaliable options are:\n");
#ifdef DEBUG
    fprintf(f, "   -A, --debug-asm           debug ASM code being debugger\n");
    fprintf(f, "   -B, --debug-bison         activate BISON debugger\n");
    fprintf(f, "   -D, --repl-disassemble    disassemble when using REPL\n");
    fprintf(f, "   -G, --debug-gc            activate GC debugger\n");
#endif
    fprintf(f, "   -h, --help                display this help and exit\n");
    fprintf(f, "   -v, --version             show version and exit\n");
    exit(exit_status);
}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
