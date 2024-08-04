#include <iostream>

#include "test_halfton.hxx"
#include "test_unicode.hxx"

void run_all_tests()
{
    test_unicode();
    test_set_default_screen();
}

int main()
{
    run_all_tests();

    std::cout << "All tests passed" << std::endl;

    return std::cout.good() ? EXIT_SUCCESS : EXIT_FAILURE;
}
