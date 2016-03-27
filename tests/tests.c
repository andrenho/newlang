#include <stdio.h>
#include "tests/minunit.h"

#include "lib/bytecode.h"
#include "lib/stack.h"
#include "lib/zoe.h"

int error = 0;
static void my_error(const char* s) {
    (void) s;
    error = 1;
}

// {{{ LOW-LEVEL STACK

static char* test_stack(void) 
{
    error = 0;

    UserFunctions uf = {
        .realloc = realloc,
        .free    = free,
        .error   = my_error,
    };
    Stack* st = stack_new(&uf);

    mu_assert("stack size == 0", stack_size(st) == 0);

    stack_push(st, (ZValue) { .type=NIL });

    mu_assert("stack size == 1 (after push)", stack_size(st) == 1);
    mu_assert("stack abs 0 = 0", stack_abs(st, 0) == 0);
    mu_assert("stack abs -1 = 0", stack_abs(st, -1) == 0);
        
    mu_assert("push & peek", stack_peek(st, -1).type == NIL);

    stack_popfree(st);
    mu_assert("stack size == 0 (after push/pop)", stack_size(st) == 0);

    mu_assert("no errors so far", error == 0);
    stack_pop(st);
    mu_assert("stack underflow", error == 1);

    stack_free(st);
    return 0;
}

// }}}

// {{{ HIGH-LEVEL STACK

static char* test_zoe_stack(void) 
{
    Zoe* Z = zoe_createvm(NULL);

    mu_assert("stack size == 0", zoe_stacksize(Z) == 0);

    double f = 3.24;
    zoe_pushnumber(Z, f);
    mu_assert("stack size == 1 (after push)", zoe_stacksize(Z) == 1);
    mu_assert("peek", zoe_peeknumber(Z) == f);
    mu_assert("pop", zoe_popnumber(Z) == f);
    mu_assert("stack size == 0 (after push/pop)", zoe_stacksize(Z) == 0);

    zoe_free(Z);
    return 0;
}

// }}}

// {{{ BYTECODE

static char* test_bytecode(void) 
{
    Bytecode* bc = bytecode_new(&default_userfunctions);

    mu_assert("size == 0", bytecode_data(bc, NULL) == 0);

    bytecode_add(bc, 0x80);

    uint8_t* buf;
    mu_assert("size == 1", bytecode_copy_data(bc, &buf) == 1);
    mu_assert("[0] = 0x80", buf[0] == 0x80);
    free(buf);

    bytecode_addf64(bc, 3.1416);

    mu_assert("size == 9", bytecode_copy_data(bc, &buf) == 9);
    mu_assert("[1] = 0xA7", buf[1] == 0xA7);
    mu_assert("[8] = 0x40", buf[8] == 0x40);
    free(buf);

    bytecode_free(bc);
    return 0;
}


static char* test_bytecode_simplecode(void)
{
    Bytecode* bc = bytecode_newfromcode(&default_userfunctions, "3.1416");

    uint8_t* buf;
    mu_assert("size == 9", bytecode_copy_data(bc, &buf) == 1);
    mu_assert("[0] = 0x03", buf[0] == 0x03); // PUSH_N - TODO
    mu_assert("[1] = 0xA7", buf[1] == 0xA7);
    mu_assert("[8] = 0x40", buf[8] == 0x40);
    free(buf);

    return 0;
}

// }}}

static char* all_tests(void)
{
    mu_run_test(test_stack);
    mu_run_test(test_zoe_stack);
    mu_run_test(test_bytecode);
    mu_run_test(test_bytecode_simplecode);
    return 0;
}

// {{{ TEST MANAGEMENT

int tests_run = 0;

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
