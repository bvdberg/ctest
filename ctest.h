/* Copyright 2011-2020 Bas van den Berg
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

#ifndef CTEST_H
#define CTEST_H

#ifdef __GNUC__
#define CTEST_IMPL_FORMAT_PRINTF(a, b) __attribute__ ((format(printf, a, b)))
#else
#define CTEST_IMPL_FORMAT_PRINTF(a, b)
#endif

#include <inttypes.h> /* intmax_t, uintmax_t, PRI* */
#include <stddef.h> /* size_t, offsetof */

typedef void (*ctest_setup_func)(void*);
typedef void (*ctest_teardown_func)(void*);

#define CTEST_IMPL_PRAGMA(x) _Pragma (#x)

#define CTEST_IMPL_COMPILER_MINIMUM_VERSION(exp_major, exp_minor, \
                                            real_major, real_minor) \
    (((real_major) > (exp_major)) \
     || ((real_major) == (exp_major) && (real_minor) >= (exp_minor)))

#if defined(__GNUC__) && !defined(__clang__)
#   define CTEST_IMPL_GCC_MINIMUM_VERSION(major, minor) \
        CTEST_IMPL_COMPILER_MINIMUM_VERSION(major, minor, \
                                            __GNUC__, __GNUC_MINOR__)
#else
#   define CTEST_IMPL_GCC_MINIMUM_VERSION(major, minor) 0
#endif

#ifdef __clang__
#   define CTEST_IMPL_CLANG_MINIMUM_VERSION(major, minor) \
        CTEST_IMPL_COMPILER_MINIMUM_VERSION(major, minor, \
                                            __clang_major__, __clang_minor__)
#else
#   define CTEST_IMPL_CLANG_MINIMUM_VERSION(major, minor) 0
#endif

#if CTEST_IMPL_GCC_MINIMUM_VERSION(4, 6) || defined(__clang__)
/* the GCC argument will work for both gcc and clang  */
#define CTEST_IMPL_DIAG_PUSH_IGNORED(w) \
    CTEST_IMPL_PRAGMA(GCC diagnostic push) \
    CTEST_IMPL_PRAGMA(GCC diagnostic ignored "-W" #w)
#define CTEST_IMPL_DIAG_POP() \
    CTEST_IMPL_PRAGMA(GCC diagnostic pop)
#elif defined(__GNUC__)
/* the push/pop functionality wasn't in gcc until 4.6, fallback to "ignored"  */
#define CTEST_IMPL_DIAG_PUSH_IGNORED(w) \
    CTEST_IMPL_PRAGMA(GCC diagnostic ignored "-W" #w)
#define CTEST_IMPL_DIAG_POP()
#else
/* leave them out entirely for non-GNUC compilers  */
#define CTEST_IMPL_DIAG_PUSH_IGNORED(w)
#define CTEST_IMPL_DIAG_POP()
#endif

#if CTEST_IMPL_GCC_MINIMUM_VERSION(4, 8)
#   define CTEST_IMPL_NO_ASAN_FUNCTION __attribute__ ((no_sanitize_address))
#   define CTEST_IMPL_NO_ASAN_VARIABLE
#elif CTEST_IMPL_CLANG_MINIMUM_VERSION(4, 0)
#   define CTEST_IMPL_NO_ASAN_FUNCTION __attribute__ ((no_sanitize_address))
#   define CTEST_IMPL_NO_ASAN_VARIABLE __attribute__ ((no_sanitize_address))
#elif CTEST_IMPL_CLANG_MINIMUM_VERSION(3, 1) && __has_feature(address_sanitizer)
#   warning "Clang 3.x AddressSanitizer may interfere with ctest, proceed with caution!"
#   define CTEST_IMPL_NO_ASAN_FUNCTION __attribute__ ((no_sanitize_address))
#   define CTEST_IMPL_NO_ASAN_VARIABLE
#   define CTEST_IMPL_CLANG_POISIONED_GLOBALS
#else
#   define CTEST_IMPL_NO_ASAN_FUNCTION
#   define CTEST_IMPL_NO_ASAN_VARIABLE
#endif

CTEST_IMPL_DIAG_PUSH_IGNORED(strict-prototypes)

struct ctest {
    unsigned int magic1; // must be first!

    const char* ssname;  // suite name
    const char* ttname;  // test name
    void (*run)();

    void* data;
    ctest_setup_func* setup;
    ctest_teardown_func* teardown;

    int skip;

    unsigned int magic2; // must be last!
};

CTEST_IMPL_DIAG_POP()

#define CTEST_IMPL_NAME(name) ctest_##name
#define CTEST_IMPL_FNAME(sname, tname) CTEST_IMPL_NAME(sname##_##tname##_run)
#define CTEST_IMPL_TNAME(sname, tname) CTEST_IMPL_NAME(sname##_##tname)
#define CTEST_IMPL_DATA_SNAME(sname) CTEST_IMPL_NAME(sname##_data)
#define CTEST_IMPL_DATA_TNAME(sname, tname) CTEST_IMPL_NAME(sname##_##tname##_data)
#define CTEST_IMPL_SETUP_FNAME(sname) CTEST_IMPL_NAME(sname##_setup)
#define CTEST_IMPL_SETUP_FPNAME(sname) CTEST_IMPL_NAME(sname##_setup_ptr)
#define CTEST_IMPL_TEARDOWN_FNAME(sname) CTEST_IMPL_NAME(sname##_teardown)
#define CTEST_IMPL_TEARDOWN_FPNAME(sname) CTEST_IMPL_NAME(sname##_teardown_ptr)

#define CTEST_IMPL_MAGIC (0xdeadbeef)
#ifdef __APPLE__
#define CTEST_IMPL_SECTION __attribute__ ((used, section ("__DATA, .ctest"), aligned(1)))
#else
#define CTEST_IMPL_SECTION __attribute__ ((used, section (".ctest"), aligned(1)))
#endif

#define CTEST_IMPL_STRUCT(sname, tname, tskip, tdata, tsetup, tteardown) \
    CTEST_IMPL_NO_ASAN_VARIABLE \
    CTEST_IMPL_SECTION \
    static struct ctest CTEST_IMPL_TNAME(sname, tname) = { \
        .magic1 = CTEST_IMPL_MAGIC, \
        .ssname=#sname, \
        .ttname=#tname, \
        .run = CTEST_IMPL_FNAME(sname, tname), \
        .data = tdata, \
        .setup = (ctest_setup_func*) tsetup, \
        .teardown = (ctest_teardown_func*) tteardown, \
        .skip = tskip, \
        .magic2 = CTEST_IMPL_MAGIC, \
    }

#define CTEST_SETUP(sname) \
    static void CTEST_IMPL_SETUP_FNAME(sname)(struct CTEST_IMPL_DATA_SNAME(sname)* data); \
    static void (*CTEST_IMPL_SETUP_FPNAME(sname))(struct CTEST_IMPL_DATA_SNAME(sname)*) = &CTEST_IMPL_SETUP_FNAME(sname); \
    static void CTEST_IMPL_SETUP_FNAME(sname)(struct CTEST_IMPL_DATA_SNAME(sname)* data)

#define CTEST_TEARDOWN(sname) \
    static void CTEST_IMPL_TEARDOWN_FNAME(sname)(struct CTEST_IMPL_DATA_SNAME(sname)* data); \
    static void (*CTEST_IMPL_TEARDOWN_FPNAME(sname))(struct CTEST_IMPL_DATA_SNAME(sname)*) = &CTEST_IMPL_TEARDOWN_FNAME(sname); \
    static void CTEST_IMPL_TEARDOWN_FNAME(sname)(struct CTEST_IMPL_DATA_SNAME(sname)* data)

#define CTEST_DATA(sname) \
    struct CTEST_IMPL_DATA_SNAME(sname); \
    static void (*CTEST_IMPL_SETUP_FPNAME(sname))(struct CTEST_IMPL_DATA_SNAME(sname)*); \
    static void (*CTEST_IMPL_TEARDOWN_FPNAME(sname))(struct CTEST_IMPL_DATA_SNAME(sname)*); \
    struct CTEST_IMPL_DATA_SNAME(sname)

#define CTEST_IMPL_CTEST(sname, tname, tskip) \
    static void CTEST_IMPL_FNAME(sname, tname)(void); \
    CTEST_IMPL_STRUCT(sname, tname, tskip, NULL, NULL, NULL); \
    static void CTEST_IMPL_FNAME(sname, tname)(void)

#define CTEST_IMPL_CTEST2(sname, tname, tskip) \
    static struct CTEST_IMPL_DATA_SNAME(sname) CTEST_IMPL_DATA_TNAME(sname, tname); \
    static void CTEST_IMPL_FNAME(sname, tname)(struct CTEST_IMPL_DATA_SNAME(sname)* data); \
    CTEST_IMPL_STRUCT(sname, tname, tskip, &CTEST_IMPL_DATA_TNAME(sname, tname), &CTEST_IMPL_SETUP_FPNAME(sname), &CTEST_IMPL_TEARDOWN_FPNAME(sname)); \
    static void CTEST_IMPL_FNAME(sname, tname)(struct CTEST_IMPL_DATA_SNAME(sname)* data)


void CTEST_LOG(const char* fmt, ...) CTEST_IMPL_FORMAT_PRINTF(1, 2);
void CTEST_ERR(const char* fmt, ...) CTEST_IMPL_FORMAT_PRINTF(1, 2);  // doesn't return

#define CTEST(sname, tname) CTEST_IMPL_CTEST(sname, tname, 0)
#define CTEST_SKIP(sname, tname) CTEST_IMPL_CTEST(sname, tname, 1)

#define CTEST2(sname, tname) CTEST_IMPL_CTEST2(sname, tname, 0)
#define CTEST2_SKIP(sname, tname) CTEST_IMPL_CTEST2(sname, tname, 1)


void assert_str(const char* exp, const char* real, const char* caller, int line);
#define ASSERT_STR(exp, real) assert_str(exp, real, __FILE__, __LINE__)

void assert_wstr(const wchar_t *exp, const wchar_t *real, const char* caller, int line);
#define ASSERT_WSTR(exp, real) assert_wstr(exp, real, __FILE__, __LINE__)

void assert_data(const unsigned char* exp, size_t expsize,
                 const unsigned char* real, size_t realsize,
                 const char* caller, int line);
#define ASSERT_DATA(exp, expsize, real, realsize) \
    assert_data(exp, expsize, real, realsize, __FILE__, __LINE__)

void assert_equal(intmax_t exp, intmax_t real, const char* caller, int line);
#define ASSERT_EQUAL(exp, real) assert_equal(exp, real, __FILE__, __LINE__)

void assert_equal_u(uintmax_t exp, uintmax_t real, const char* caller, int line);
#define ASSERT_EQUAL_U(exp, real) assert_equal_u(exp, real, __FILE__, __LINE__)

void assert_not_equal(intmax_t exp, intmax_t real, const char* caller, int line);
#define ASSERT_NOT_EQUAL(exp, real) assert_not_equal(exp, real, __FILE__, __LINE__)

void assert_not_equal_u(uintmax_t exp, uintmax_t real, const char* caller, int line);
#define ASSERT_NOT_EQUAL_U(exp, real) assert_not_equal_u(exp, real, __FILE__, __LINE__)

void assert_interval(intmax_t exp1, intmax_t exp2, intmax_t real, const char* caller, int line);
#define ASSERT_INTERVAL(exp1, exp2, real) assert_interval(exp1, exp2, real, __FILE__, __LINE__)

void assert_null(void* real, const char* caller, int line);
#define ASSERT_NULL(real) assert_null((void*)real, __FILE__, __LINE__)

void assert_not_null(const void* real, const char* caller, int line);
#define ASSERT_NOT_NULL(real) assert_not_null(real, __FILE__, __LINE__)

void assert_true(int real, const char* caller, int line);
#define ASSERT_TRUE(real) assert_true(real, __FILE__, __LINE__)

void assert_false(int real, const char* caller, int line);
#define ASSERT_FALSE(real) assert_false(real, __FILE__, __LINE__)

void assert_fail(const char* caller, int line);
#define ASSERT_FAIL() assert_fail(__FILE__, __LINE__)

void assert_dbl_near(double exp, double real, double tol, const char* caller, int line);
#define ASSERT_DBL_NEAR(exp, real) assert_dbl_near(exp, real, 1e-4, __FILE__, __LINE__)
#define ASSERT_DBL_NEAR_TOL(exp, real, tol) assert_dbl_near(exp, real, tol, __FILE__, __LINE__)

void assert_dbl_far(double exp, double real, double tol, const char* caller, int line);
#define ASSERT_DBL_FAR(exp, real) assert_dbl_far(exp, real, 1e-4, __FILE__, __LINE__)
#define ASSERT_DBL_FAR_TOL(exp, real, tol) assert_dbl_far(exp, real, tol, __FILE__, __LINE__)

#ifdef CTEST_MAIN

#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <wchar.h>

static size_t ctest_errorsize;
static char* ctest_errormsg;
#define MSG_SIZE 4096
static char ctest_errorbuffer[MSG_SIZE];
static jmp_buf ctest_err;
static int color_output = 1;
static const char* suite_name;

typedef int (*ctest_filter_func)(struct ctest*);

#define ANSI_BLACK    "\033[0;30m"
#define ANSI_RED      "\033[0;31m"
#define ANSI_GREEN    "\033[0;32m"
#define ANSI_YELLOW   "\033[0;33m"
#define ANSI_BLUE     "\033[0;34m"
#define ANSI_MAGENTA  "\033[0;35m"
#define ANSI_CYAN     "\033[0;36m"
#define ANSI_GREY     "\033[0;37m"
#define ANSI_DARKGREY "\033[01;30m"
#define ANSI_BRED     "\033[01;31m"
#define ANSI_BGREEN   "\033[01;32m"
#define ANSI_BYELLOW  "\033[01;33m"
#define ANSI_BBLUE    "\033[01;34m"
#define ANSI_BMAGENTA "\033[01;35m"
#define ANSI_BCYAN    "\033[01;36m"
#define ANSI_WHITE    "\033[01;37m"
#define ANSI_NORMAL   "\033[0m"

CTEST(suite, test) { }

static void vprint_errormsg(const char* const fmt, va_list ap) CTEST_IMPL_FORMAT_PRINTF(1, 0);
static void print_errormsg(const char* const fmt, ...) CTEST_IMPL_FORMAT_PRINTF(1, 2);

static void vprint_errormsg(const char* const fmt, va_list ap) {
    // (v)snprintf returns the number that would have been written
    const int ret = vsnprintf(ctest_errormsg, ctest_errorsize, fmt, ap);
    if (ret < 0) {
        ctest_errormsg[0] = 0x00;
    } else {
        const size_t size = (size_t) ret;
        const size_t s = (ctest_errorsize <= size ? size -ctest_errorsize : size);
        // ctest_errorsize may overflow at this point
        ctest_errorsize -= s;
        ctest_errormsg += s;
    }
}

static void print_errormsg(const char* const fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    vprint_errormsg(fmt, argp);
    va_end(argp);
}

static void msg_start(const char* color, const char* title) {
    if (color_output) {
        print_errormsg("%s", color);
    }
    print_errormsg("  %s: ", title);
}

static void msg_end(void) {
    if (color_output) {
        print_errormsg(ANSI_NORMAL);
    }
    print_errormsg("\n");
}

void CTEST_LOG(const char* fmt, ...)
{
    va_list argp;
    msg_start(ANSI_BLUE, "LOG");

    va_start(argp, fmt);
    vprint_errormsg(fmt, argp);
    va_end(argp);

    msg_end();
}

CTEST_IMPL_DIAG_PUSH_IGNORED(missing-noreturn)

void CTEST_ERR(const char* fmt, ...)
{
    va_list argp;
    msg_start(ANSI_YELLOW, "ERR");

    va_start(argp, fmt);
    vprint_errormsg(fmt, argp);
    va_end(argp);

    msg_end();
    longjmp(ctest_err, 1);
}

CTEST_IMPL_DIAG_POP()

void assert_str(const char* exp, const char*  real, const char* caller, int line) {
    if ((exp == NULL && real != NULL) ||
        (exp != NULL && real == NULL) ||
        (exp && real && strcmp(exp, real) != 0)) {
        CTEST_ERR("%s:%d  expected '%s', got '%s'", caller, line, exp, real);
    }
}

void assert_wstr(const wchar_t *exp, const wchar_t *real, const char* caller, int line) {
    if ((exp == NULL && real != NULL) ||
        (exp != NULL && real == NULL) ||
        (exp && real && wcscmp(exp, real) != 0)) {
        CTEST_ERR("%s:%d  expected '%ls', got '%ls'", caller, line, exp, real);
    }
}

void assert_data(const unsigned char* exp, size_t expsize,
                 const unsigned char* real, size_t realsize,
                 const char* caller, int line) {
    size_t i;
    if (expsize != realsize) {
        CTEST_ERR("%s:%d  expected %" PRIuMAX " bytes, got %" PRIuMAX, caller, line, (uintmax_t) expsize, (uintmax_t) realsize);
    }
    for (i=0; i<expsize; i++) {
        if (exp[i] != real[i]) {
            CTEST_ERR("%s:%d expected 0x%02x at offset %" PRIuMAX " got 0x%02x",
                caller, line, exp[i], (uintmax_t) i, real[i]);
        }
    }
}

void assert_equal(intmax_t exp, intmax_t real, const char* caller, int line) {
    if (exp != real) {
        CTEST_ERR("%s:%d  expected %" PRIdMAX ", got %" PRIdMAX, caller, line, exp, real);
    }
}

void assert_equal_u(uintmax_t exp, uintmax_t real, const char* caller, int line) {
    if (exp != real) {
        CTEST_ERR("%s:%d  expected %" PRIuMAX ", got %" PRIuMAX, caller, line, exp, real);
    }
}

void assert_not_equal(intmax_t exp, intmax_t real, const char* caller, int line) {
    if ((exp) == (real)) {
        CTEST_ERR("%s:%d  should not be %" PRIdMAX, caller, line, real);
    }
}

void assert_not_equal_u(uintmax_t exp, uintmax_t real, const char* caller, int line) {
    if ((exp) == (real)) {
        CTEST_ERR("%s:%d  should not be %" PRIuMAX, caller, line, real);
    }
}

void assert_interval(intmax_t exp1, intmax_t exp2, intmax_t real, const char* caller, int line) {
    if (real < exp1 || real > exp2) {
        CTEST_ERR("%s:%d  expected %" PRIdMAX "-%" PRIdMAX ", got %" PRIdMAX, caller, line, exp1, exp2, real);
    }
}

void assert_dbl_near(double exp, double real, double tol, const char* caller, int line) {
    double diff = exp - real;
    double absdiff = diff;
    /* avoid using fabs and linking with a math lib */
    if(diff < 0) {
      absdiff *= -1;
    }
    if (absdiff > tol) {
        CTEST_ERR("%s:%d  expected %0.3e, got %0.3e (diff %0.3e, tol %0.3e)", caller, line, exp, real, diff, tol);
    }
}

void assert_dbl_far(double exp, double real, double tol, const char* caller, int line) {
    double diff = exp - real;
    double absdiff = diff;
    /* avoid using fabs and linking with a math lib */
    if(diff < 0) {
      absdiff *= -1;
    }
    if (absdiff <= tol) {
        CTEST_ERR("%s:%d  expected %0.3e, got %0.3e (diff %0.3e, tol %0.3e)", caller, line, exp, real, diff, tol);
    }
}

void assert_null(void* real, const char* caller, int line) {
    if ((real) != NULL) {
        CTEST_ERR("%s:%d  should be NULL", caller, line);
    }
}

void assert_not_null(const void* real, const char* caller, int line) {
    if (real == NULL) {
        CTEST_ERR("%s:%d  should not be NULL", caller, line);
    }
}

void assert_true(int real, const char* caller, int line) {
    if ((real) == 0) {
        CTEST_ERR("%s:%d  should be true", caller, line);
    }
}

void assert_false(int real, const char* caller, int line) {
    if ((real) != 0) {
        CTEST_ERR("%s:%d  should be false", caller, line);
    }
}

void assert_fail(const char* caller, int line) {
    CTEST_ERR("%s:%d  shouldn't come here", caller, line);
}


static int suite_all(struct ctest* t) {
    (void) t; // fix unused parameter warning
    return 1;
}

static int suite_filter(struct ctest* t) {
    return strncmp(suite_name, t->ssname, strlen(suite_name)) == 0;
}

static uint64_t getCurrentTime(void) {
    struct timeval now;
    gettimeofday(&now, NULL);
    uint64_t now64 = (uint64_t) now.tv_sec;
    now64 *= 1000000;
    now64 += ((uint64_t) now.tv_usec);
    return now64;
}

static void color_print(const char* color, const char* text) {
    if (color_output)
        printf("%s%s"ANSI_NORMAL"\n", color, text);
    else
        printf("%s\n", text);
}

#ifdef CTEST_SEGFAULT
#include <signal.h>
static void sighandler(int signum)
{
    const char msg_color[] = ANSI_BRED "[SIGSEGV: Segmentation fault]" ANSI_NORMAL "\n";
    const char msg_nocolor[] = "[SIGSEGV: Segmentation fault]\n";

    const char* msg = color_output ? msg_color : msg_nocolor;
    write(STDOUT_FILENO, msg, strlen(msg));

    /* "Unregister" the signal handler and send the signal back to the process
     * so it can terminate as expected */
    signal(signum, SIG_DFL);
    kill(getpid(), signum);
}
#endif

#ifdef CTEST_IMPL_CLANG_POISIONED_GLOBALS
#include <sanitizer/asan_interface.h>
#endif

static struct ctest* ctest_adjacent_test(const struct ctest* test,
                                         int direction, size_t offset)
{
    const volatile unsigned int* magic = (const unsigned int*)
                                         ((const char*)
                                          (test + direction) + offset);

#if CTEST_IMPL_POISIONED_GLOBALS
    while (ASAN_UNPOISON_MEMORY_REGION(magic, sizeof(*magic)), *magic == 0) {
        magic += direction;
    }
#endif

CTEST_IMPL_DIAG_PUSH_IGNORED(cast-qual)
    return *magic == CTEST_IMPL_MAGIC ? (struct ctest*)
                                        ((char*)magic - offset)
                                      : NULL;
CTEST_IMPL_DIAG_POP()
}

static struct ctest* ctest_next_test(const struct ctest* test)
{
    return ctest_adjacent_test(test, 1, offsetof(struct ctest, magic1));
}

static struct ctest* ctest_prev_test(const struct ctest* test)
{
    return ctest_adjacent_test(test, -1, offsetof(struct ctest, magic2));
}

CTEST_IMPL_NO_ASAN_FUNCTION
int ctest_main(int argc, const char *argv[])
{
    static int total = 0;
    static int num_ok = 0;
    static int num_fail = 0;
    static int num_skip = 0;
    static int idx = 1;
    static ctest_filter_func filter = suite_all;

#ifdef CTEST_SEGFAULT
    signal(SIGSEGV, sighandler);
#endif

    if (argc == 2) {
        suite_name = argv[1];
        filter = suite_filter;
    }
#ifdef CTEST_NO_COLORS
    color_output = 0;
#else
    color_output = isatty(1);
#endif
    uint64_t t1 = getCurrentTime();

    struct ctest* ctest_begin = &CTEST_IMPL_TNAME(suite, test);
    while (1) {
        struct ctest* t = ctest_prev_test(ctest_begin);
        if (t == NULL) break;
        ctest_begin = t;
    }

    static struct ctest* test;
    for (test = ctest_begin; test != NULL; test = ctest_next_test(test)) {
        if (test == &CTEST_IMPL_TNAME(suite, test)) continue;
        if (filter(test)) total++;
    }

    for (test = ctest_begin; test != NULL; test = ctest_next_test(test)) {
        if (test == &CTEST_IMPL_TNAME(suite, test)) continue;
        if (filter(test)) {
            ctest_errorbuffer[0] = 0;
            ctest_errorsize = MSG_SIZE-1;
            ctest_errormsg = ctest_errorbuffer;
            printf("TEST %d/%d %s:%s ", idx, total, test->ssname, test->ttname);
            fflush(stdout);
            if (test->skip) {
                color_print(ANSI_BYELLOW, "[SKIPPED]");
                num_skip++;
            } else {
                int result = setjmp(ctest_err);
                if (result == 0) {
                    if (test->setup && *test->setup) (*test->setup)(test->data);
                    if (test->data)
                        test->run(test->data);
                    else
                        test->run();
                    if (test->teardown && *test->teardown) (*test->teardown)(test->data);
                    // if we got here it's ok
#ifdef CTEST_COLOR_OK
                    color_print(ANSI_BGREEN, "[OK]");
#else
                    printf("[OK]\n");
#endif
                    num_ok++;
                } else {
                    color_print(ANSI_BRED, "[FAIL]");
                    num_fail++;
                }
                if (ctest_errorsize != MSG_SIZE-1) printf("%s", ctest_errorbuffer);
            }
            idx++;
        }
    }
    uint64_t t2 = getCurrentTime();

    const char* color = (num_fail) ? ANSI_BRED : ANSI_GREEN;
    char results[80];
    snprintf(results, sizeof(results), "RESULTS: %d tests (%d ok, %d failed, %d skipped) ran in %" PRIu64 " ms", total, num_ok, num_fail, num_skip, (t2 - t1)/1000);
    color_print(color, results);
    return num_fail;
}

#endif

#endif

