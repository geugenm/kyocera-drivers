#include "minunit.h"

int tests_run = 0;

const int foo = 7;
const int bar = 5;

char* test_foo()
{
    mu_assert("error, foo != 7", foo == 7);
    return 0;
}

char* test_bar()
{
    mu_assert("error, bar != 5", bar == 5);
    return 0;
}