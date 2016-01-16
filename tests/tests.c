#include <stdio.h>
#include <string.h>
#include "tests/minunit.h"

#include "lib/bytecode.h"
#include "lib/opcode.h"
#include "lib/stack.h"
#include "lib/zoe.h"

#pragma GCC diagnostic ignored "-Wfloat-equal"

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


static char* test_zoe_stack_order(void)
{
    Zoe* Z = zoe_createvm(NULL);

    zoe_pushnumber(Z, 1);
    zoe_pushnumber(Z, 2);
    zoe_pushnumber(Z, 3);
    mu_assert("stack size == 3", zoe_stacksize(Z) == 3);
    mu_assert("pop == 3", zoe_popnumber(Z) == 3);
    mu_assert("pop == 3", zoe_popnumber(Z) == 2);
    mu_assert("pop == 3", zoe_popnumber(Z) == 1);
    mu_assert("stack size == 0", zoe_stacksize(Z) == 0);

    zoe_free(Z);
    return 0;
}


static char* test_zoe_string(void)
{
    Zoe* Z = zoe_createvm(NULL);
    
    zoe_pushstring(Z, "hello world");
    mu_assert("stack size == 1 (after push)", zoe_stacksize(Z) == 1);
    mu_assert("peek", strcmp(zoe_peekstring(Z), "hello world") == 0);

    char* buf = zoe_popstring(Z);
    mu_assert("pop", strcmp(buf, "hello world") == 0);
    free(buf);

    mu_assert("stack size == 0 (after push/pop)", zoe_stacksize(Z) == 0);
    zoe_free(Z);

    return 0;
}


// }}}

// {{{ BYTECODE

static uint8_t expected[] = {
    0x90, 0x6F, 0x65, 0x20, 0xEB, 0x00, 0x01, 0x00,     // header
    0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // code_pos
    0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // code_sz
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // data_pos
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // data_sz
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // debug_pos
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // debug_sz
    PUSH_N, 0xA7, 0xE8, 0x48, 0x2E, 0xFF, 0x21, 0x09, 0x40,  // PUSH_N 3.1416
};

static char* test_bytecode_gen(void) 
{
    Bytecode* bc = bytecode_new(&default_userfunctions);

    bytecode_addcode(bc, PUSH_N);
    bytecode_addcodef64(bc, 3.1416);

    uint8_t* found;
    size_t sz = bytecode_generatezb(bc, &found);

    mu_assert("BZ size", sz == sizeof expected);
    for(size_t i=0; i<sz; ++i) {
        static char buf[100];
        sprintf(buf, "%zuth byte", i);
        mu_assert(buf, expected[i] == found[i]);
    }
    free(found);

    bytecode_free(bc);
    
    return 0;
}


static char* test_bytecode_import(void)
{
    Bytecode* bc = bytecode_newfromzb(&default_userfunctions, expected, sizeof expected);

    mu_assert("version minor", bc->version_minor == 0x1);
    mu_assert("code_sz", bc->code_sz == 9);
    mu_assert("code[0]", bc->code[0] == PUSH_N);
    mu_assert("code[1]", bc->code[1] == 0xA7);
    mu_assert("code[8]", bc->code[8] == 0x40);

    bytecode_free(bc);

    return 0;
}


static char* test_bytecode_simplecode(void)
{
    Bytecode* bc = bytecode_newfromcode(&default_userfunctions, "3.1416");

    uint8_t* found;
    size_t sz = bytecode_generatezb(bc, &found);

    mu_assert("BZ size", sz == sizeof expected);
    for(size_t i=0; i<sz; ++i) {
        static char buf[100];
        sprintf(buf, "%zuth byte", i);
        mu_assert(buf, expected[i] == found[i]);
    }
    free(found);

    bytecode_free(bc);

    return 0;
}

// }}}

// {{{ ZOE EXECUTION

static char* test_execution(void) 
{
    Zoe* Z = zoe_createvm(NULL);

    // load code
    zoe_eval(Z, "42");
    mu_assert("eval pushed into stack", zoe_stacksize(Z) == 1);
    mu_assert("type == function", zoe_peektype(Z) == FUNCTION);

    // execute code
    zoe_call(Z, 0);
    mu_assert("result pushed into stack", zoe_stacksize(Z) == 1);
    mu_assert("type == function", zoe_peektype(Z) == NUMBER);
    mu_assert("return 42", zoe_popnumber(Z) == 42);

    zoe_free(Z);

    return 0;
}

static char* test_inspect(void)
{
    Zoe* Z = zoe_createvm(NULL);

    zoe_eval(Z, "42");
    zoe_call(Z, 0);
    zoe_inspect(Z);

    char* r = zoe_popstring(Z);
    mu_assert("-> 42", strcmp(r, "42") == 0);

    zoe_free(Z);
    return 0;
}

// }}}

static char* all_tests(void)
{
    mu_run_test(test_stack);
    mu_run_test(test_zoe_stack);
    mu_run_test(test_zoe_stack_order);
    mu_run_test(test_zoe_string);
    mu_run_test(test_bytecode_gen);
    mu_run_test(test_bytecode_import);
    mu_run_test(test_bytecode_simplecode);
    mu_run_test(test_execution);
    mu_run_test(test_inspect);
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
