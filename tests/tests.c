#include <stdio.h>
#include "tests/minunit.h"

#include "lib/stack.h"

// {{{ STACK

static char* test_stack(void) 
{
    Stack* st = stack_new();

    mu_assert("stack size == 0", stack_size(st) == 0);

    stack_free(st);
    return 0;
}

// }}}

// {{{ TEST MANAGEMENT

int tests_run = 0;

static char* all_tests(void)
{
    mu_run_test(test_stack);
    return 0;
}


int main(void) 
{
    char *result = all_tests();
    if(result != 0) {
        printf("\033[1;31merror: %s\033[0m\n", result);
    } else {
        printf("\033[1;32mALL TESTS PASSED!\033[0m\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}

// }}}

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
