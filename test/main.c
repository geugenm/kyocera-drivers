#include "example.h"
#include "minunit.h"
#include "test_halfton.h"
#include "test_unicode.h"
#include <stdio.h>

static char* all_tests()
{
    mu_run_test(test_foo);
    mu_run_test(test_bar);
    mu_run_test(test_unicode);
    mu_run_test(test_setDefaultScreen);
    return 0;
}

int main(int argc, char** argv)
{
    char* result = all_tests();
    if (result != 0)
    {
        printf("%s\n", result);
    }
    else
    {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}