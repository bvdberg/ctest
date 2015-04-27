#include "ctest.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define __func__ __FUNCTION__
#endif

CTEST(suite_vs1, test1) {
  printf("ok!\n");
}

CTEST(suite_vs2, test1) {
  printf("ok2!\n");
}

// A test suite with a setup/teardown function
// This is converted into a struct that's automatically passed to all tests in the suite
CTEST_DATA(memtest5) {
    unsigned char* buffer;
};

// Optional setup function for suite, called before every test in suite
CTEST_SETUP(memtest5) {
    CTEST_LOG("%s() data=%p buffer=%p", __func__, data, data->buffer);
    data->buffer = (unsigned char*)malloc(1024);
}

// Optional teardown function for suite, called after every test in suite
CTEST_TEARDOWN(memtest5) {
    CTEST_LOG("%s() data=%p buffer=%p", __func__, data, data->buffer);
    if (data->buffer) free(data->buffer);
}

// These tests are called with the struct* (named data) as argument
CTEST2(memtest5, test1) {
    CTEST_LOG("%s()  data=%p  buffer=%p", __func__, data, data->buffer);
}
