// This amazingly simple test unit framework comes from
// <http://www.jera.com/techinfo/jtns/jtn002.html>

#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) printf("\033[1;34m[%s]\033[0m\n", #test); \
    do { char *message = test(); tests_run++; \
    if (message) return message; } while (0)
extern int tests_run;

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
