/* Copyright 2011-2023 Bas van den Berg
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include "ctest.h"
#include "ctest_fmt.h"

static char buf[1024];

// basic test without setup/teardown
CTEST(suite1, test1) {
}

// there are many different ASSERT macro's (see ctest.h)
CTEST(suite1, test2) {
    ASSERT_EQUAL(1,2);
}

CTEST(suite1, test2_formatted) {
    ASSERT_EQUAL_D(1,2, CTEST_DESCRIPTION_FORMAT(buf, sizeof(buf), "%s", "formatted message"));
}

CTEST(suite2, test1) {
    ASSERT_STR("foo", "bar");
}

CTEST(suite2, test1_descr) {
    ASSERT_STR_D("foo", "bar", "message");
}

CTEST(suite3, test3) {
}


// A test suite with a setup/teardown function
// This is converted into a struct that's automatically passed to all tests in the suite
CTEST_DATA(memtest) {
    unsigned char* buffer;
};

// Optional setup function for suite, called before every test in suite
CTEST_SETUP(memtest) {
    CTEST_LOG("%s() data=%p buffer=%p", __func__, (void*)data, (void*)data->buffer);
    data->buffer = (unsigned char*)malloc(1024);
}

// Optional teardown function for suite, called after every test in suite
CTEST_TEARDOWN(memtest) {
    CTEST_LOG("%s() data=%p buffer=%p", __func__, (void*)data, (void*)data->buffer);
    if (data->buffer) free(data->buffer);
}

// These tests are called with the struct* (named data) as argument
CTEST2(memtest, test1) {
    CTEST_LOG("%s()  data=%p  buffer=%p", __func__, (void*)data, (void*)data->buffer);
}

CTEST2_SKIP(memtest, test3) {
    (void)data;
    ASSERT_FAIL();
}

CTEST2(memtest, test2) {
    CTEST_LOG("%s()  data=%p  buffer=%p", __func__, (void*)data, (void*)data->buffer);
    ASSERT_FAIL();
}

CTEST2(memtest, test2_descr) {
    CTEST_LOG("%s()  data=%p  buffer=%p", __func__, (void*)data, (void*)data->buffer);
    ASSERT_FAIL_D("message");
}


CTEST_DATA(fail) {
    int unused;
};

// Asserts can also be used in setup/teardown functions
CTEST_SETUP(fail) {
    (void)data;
    ASSERT_FAIL();
}

CTEST2(fail, test1) {
    (void)data;
}



CTEST_DATA(weaklinkage) {
    int number;
};

// This suite has data, but no setup/teardown
CTEST2(weaklinkage, test1) {
    (void)data;
    CTEST_LOG("%s()", __func__);
}

CTEST2(weaklinkage, test2) {
    (void)data;
    CTEST_LOG("%s()", __func__);
}


CTEST_DATA(nosetup) {
    int value;
};

CTEST_TEARDOWN(nosetup) {
    (void)data;
    CTEST_LOG("%s()", __func__);
}

CTEST2(nosetup, test1) {
    (void)data;
    CTEST_LOG("%s()", __func__);
}


// more ASSERT examples
CTEST(ctest, test_assert_str) {
    ASSERT_STR("foo", "foo");
    ASSERT_STR("foo", "bar");
}

// more ASSERT examples
CTEST(ctest, test_assert_str_descr) {
    ASSERT_STR_D("foo", "foo", "message");
    ASSERT_STR_D("foo", "bar", "message");
}


CTEST(ctest, test_assert_equal) {
    ASSERT_EQUAL(123, 123);
    ASSERT_EQUAL(123, 456);
}

CTEST(ctest, test_assert_equal_descr) {
    ASSERT_EQUAL_D(123, 123, "message");
    ASSERT_EQUAL_D(123, 456, "message");
}

CTEST(ctest, test_assert_not_equal) {
    ASSERT_NOT_EQUAL(123, 456);
    ASSERT_NOT_EQUAL(123, 123);
}

CTEST(ctest, test_assert_not_equal_descr) {
    ASSERT_NOT_EQUAL_D(123, 456, "message");
    ASSERT_NOT_EQUAL_D(123, 123, "message");
}

CTEST(ctest, test_assert_interval) {
    ASSERT_INTERVAL(10, 20, 15);
    ASSERT_INTERVAL(1000, 2000, 3000);
}

CTEST(ctest, test_assert_interval_descr) {
    ASSERT_INTERVAL_D(10, 20, 15, "message");
    ASSERT_INTERVAL_D(1000, 2000, 3000, "message");
}

CTEST(ctest, test_assert_null) {
    ASSERT_NULL(NULL);
    ASSERT_NULL((void*)0xdeadbeef);
}

CTEST(ctest, test_assert_null_descr) {
    ASSERT_NULL_D(NULL, "message");
    ASSERT_NULL_D((void*)0xdeadbeef, "message");
}

CTEST(ctest, test_assert_not_null_const) {
    ASSERT_NOT_NULL((const char*)"hallo");
    ASSERT_NOT_NULL(NULL);
}

CTEST(ctest, test_assert_not_null_const_descr) {
    ASSERT_NOT_NULL_D((const char*)"hallo", "message");
    ASSERT_NOT_NULL_D(NULL, "message");
}

CTEST(ctest, test_assert_not_null) {
    ASSERT_NOT_NULL((void*)0xdeadbeef);
    ASSERT_NOT_NULL(NULL);
}

CTEST(ctest, test_assert_true) {
    ASSERT_TRUE(1);
    ASSERT_TRUE(0);
}

CTEST(ctest, test_assert_true_descr) {
    ASSERT_TRUE_D(1, "message");
    ASSERT_TRUE_D(0, "message");
}

CTEST(ctest, test_assert_false) {
    ASSERT_FALSE(0);
    ASSERT_FALSE(1);
}

CTEST(ctest, test_assert_false_descr) {
    ASSERT_FALSE_D(0, "message");
    ASSERT_FALSE_D(1, "message");
}

CTEST_SKIP(ctest, test_skip) {
    ASSERT_FAIL();
}

CTEST(ctest, test_assert_fail) {
    ASSERT_FAIL();
}

/* Test that NULL-strings won't result in segv */
CTEST(ctest, test_null_null) {
    ASSERT_STR(NULL, NULL);
}

CTEST(ctest, test_null_string) {
    ASSERT_STR(NULL, "shouldfail");
}

CTEST(ctest, test_string_null) {
    ASSERT_STR("shouldfail", NULL);
}

CTEST(ctest, test_string_diff_ptrs) {
    const char *str = "abc\0abc";
    ASSERT_STR(str, str+4);
}

CTEST(ctest, test_large_numbers) {
    unsigned long exp = 4200000000u;
    ASSERT_EQUAL_U(exp, 4200000000u);
    ASSERT_NOT_EQUAL_U(exp, 1200000000u);
}

CTEST(ctest, test_large_numbers_eq_descr) {
    ASSERT_EQUAL_U_D(4200000000u, 1200000000u, "message");
}

CTEST(ctest, test_large_numbers_noneq_descr) {
    ASSERT_NOT_EQUAL_U_D(1200000000u, 1200000000u, "message");
}

CTEST(ctest, test_ctest_err) {
    CTEST_ERR("error log");
}

CTEST(ctest, test_dbl_near) {
    double a = 0.000111;
    ASSERT_DBL_NEAR(0.0001, a);
}

CTEST(ctest, test_dbl_near_descr) {
    double a = 0.000111;
    ASSERT_DBL_NEAR_D(0.0001, a, "message");
}

CTEST(ctest, test_dbl_near_tol) {
    double a = 0.000111;
    ASSERT_DBL_NEAR_TOL(0.0001, a, 1e-5); /* will fail */
}

CTEST(ctest, test_dbl_near_tol_descr) {
    double a = 0.000111;
    ASSERT_DBL_NEAR_TOL_D(0.0001, a, 1e-5, "message"); /* will fail */
}

CTEST(ctest, test_dbl_far) {
    double a = 1.1;
    ASSERT_DBL_FAR(1., a);
    ASSERT_DBL_FAR_TOL(1., a, 0.01);
}

CTEST(ctest, test_dbl_far_descr) {
    double a = 1.1;
    ASSERT_DBL_FAR_D(1.1, a, "message");
}

CTEST(ctest, test_dbl_far_descr2) {
    double a = 1.1;
    ASSERT_DBL_FAR_TOL_D(1.091, a, 0.01, "message");
}

CTEST(ctest, test_assert_compare) {
    ASSERT_LT(123, 456);
    ASSERT_GE(123, 123);
    ASSERT_GT(99, 100);
}

CTEST(ctest, test_assert_compare_descr) {
    ASSERT_LT_D(123, 456, "message");
    ASSERT_GE_D(123, 123, "message");
    ASSERT_GT_D(99, 100, "message");
}


CTEST(ctest, test_dbl_near2) {
    float a = 0.000001000003f;
    ASSERT_FLT_NEAR(0.000001f, a);  /* ok, uses float epsilon -1e-5 */
    ASSERT_DBL_NEAR_TOL(0.000001, a, -1e-5); /* ok, tol < 0 = relative err (epsilon) */
    ASSERT_DBL_NEAR(0.000001, a);  /* fail, tol = -1e-12 (epsilon) */
}

CTEST(ctest, test_dbl_near2_descr) {
    float a = 0.000001000003f;
    ASSERT_FLT_NEAR_D(0.000001f, a, "message");  /* ok, uses float epsilon -1e-5 */
    ASSERT_DBL_NEAR_TOL_D(0.000001, a, -1e-5, "message"); /* ok, tol < 0 = relative err (epsilon) */
    ASSERT_DBL_NEAR_D(0.000001, a, "message");  /* fail, tol = -1e-12 (epsilon) */
}

CTEST(ctest, test_dbl_compare) {
    float a = 0.000001000003f;
    ASSERT_DBL_LT(0.000001, a);
    ASSERT_DBL_GT(0.000001, a);  /* fail */
}

CTEST(ctest, test_dbl_compare_descr) {
    float a = 0.000001000003f;
    ASSERT_DBL_LT_D(0.000001, a, "message");
    ASSERT_DBL_GT_D(0.000001, a, "message");  /* fail */
}

CTEST(ctest, test_str_contains) {
    ASSERT_NOT_STR("Hello", "World");
    ASSERT_STRSTR("Hello", "ello");
    ASSERT_NOT_STRSTR("Hello", "ello");
}
