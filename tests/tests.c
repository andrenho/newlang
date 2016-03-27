#include <stdio.h>
#include "tests/minunit.h"

int tests_run = 0;

static char* test_basic(void) 
{
    mu_assert("1 == 2", 1 == 2);
    return 0;
}

static char* all_tests(void)
{
    mu_run_test(test_basic);
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

// vim: ts=4:sw=4:sts=4:expandtab:foldmethod=marker:syntax=c
