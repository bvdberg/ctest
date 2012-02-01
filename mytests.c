#include <unistd.h>
#include "ctest.h"

CTEST(suite1, test1) {
    usleep(2000);
}

CTEST(suite1, test2) {
    ASSERT_EQUAL(1,2);
}

CTEST(suite2, test1) {
    ASSERT_STR("foo", "bar");
}

CTEST(suite3, test3) {
}


CTEST_DATA(memtest) {
    unsigned char* buffer;
};

CTEST_SETUP(memtest) {
    CTEST_LOG("%s() data=%p buffer=%p", __func__, data, data->buffer);
    data->buffer = (unsigned char*)malloc(1024);
}

CTEST_TEARDOWN(memtest) {
    CTEST_LOG("%s() data=%p buffer=%p", __func__, data, data->buffer);
    if (data->buffer) free(data->buffer);
}

CTEST2(memtest, test1) {
    CTEST_LOG("%s()  data=%p  buffer=%p", __func__, data, data->buffer);
}

CTEST2_SKIP(memtest, test3) {
    ASSERT_FAIL();
}

CTEST2(memtest, test2) {
    CTEST_LOG("%s()  data=%p  buffer=%p", __func__, data, data->buffer);
    
    ASSERT_FAIL();
}


CTEST_DATA(fail) {};

CTEST_SETUP(fail) {
    ASSERT_FAIL();
}

CTEST2(fail, test1) {}



CTEST_DATA(weaklinkage) {
    int number;
};

CTEST2(weaklinkage, test1) {
    CTEST_LOG("%s()", __func__);
}

CTEST2(weaklinkage, test2) {
    CTEST_LOG("%s()", __func__);
}


CTEST_DATA(nosetup) {
    int value;
};

CTEST_TEARDOWN(nosetup) {
    CTEST_LOG("%s()", __func__);
}

CTEST2(nosetup, test1) {
    CTEST_LOG("%s()", __func__);
}


CTEST(ctest, test_assert_str) {
    ASSERT_STR("foo", "foo");
    ASSERT_STR("foo", "bar");
}

CTEST(ctest, test_assert_equal) {
    ASSERT_EQUAL(123, 123);
    ASSERT_EQUAL(123, 456);
}

CTEST(ctest, test_assert_not_equal) {
    ASSERT_NOT_EQUAL(123, 456);
    ASSERT_NOT_EQUAL(123, 123);
}


CTEST(ctest, test_assert_null) {
    ASSERT_NULL(NULL);
    ASSERT_NULL((void*)0xdeadbeef);
}

CTEST(ctest, test_assert_not_null) {
    ASSERT_NOT_NULL((void*)0xdeadbeef);
    ASSERT_NOT_NULL(NULL);
}

CTEST(ctest, test_assert_true) {
    ASSERT_TRUE(1);
    ASSERT_TRUE(0);
}

CTEST(ctest, test_assert_false) {
    ASSERT_FALSE(0);
    ASSERT_FALSE(1);
}

CTEST_SKIP(ctest, test_skip) {
    ASSERT_FAIL();
}

CTEST(ctest, test_assert_fail) {
    ASSERT_FAIL();
}

