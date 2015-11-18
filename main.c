#include <stdio.h>

#define CTEST_MAIN
// uncomment line below to get nicer logging on segfaults
#define CTEST_SEGFAULT
#define CTEST_USE_COLORS
#include "ctest.h"

int main(int argc, const char *argv[])
{
    int result = ctest_main(argc, argv);

    printf("\nNOTE: some tests will fail, just to show how ctest works! ;)\n");
    return result;
}

