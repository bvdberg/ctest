/* Copyright 2011,2012 Bas van den Berg
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

#include <stdarg.h>

#define CTEST_MAGIC (0xdeadbeef)
#define Test_Section __attribute__ ((unused,section (".ctest")))

void CTEST_LOG(char *fmt, ...);

typedef void (*SetupFunc)(void*);
typedef void (*TearDownFunc)(void*);

struct ctest {
    const char* ssname;  // suite name
    const char* ttname;  // test name
    void (*run)();
    int skip;

    void* data;
    SetupFunc setup;
    TearDownFunc teardown;

    unsigned int magic;
};

extern int ctest_errorsize;
extern char* ctest_errormsg;

#define __FNAME(sname, tname) __ctest_##sname##_##tname##_run
#define __TNAME(sname, tname) __ctest_##sname##_##tname

#define __CTEST_STRUCT(sname, tname, _skip, __data, __setup, __teardown) \
    struct ctest __TNAME(sname, tname) Test_Section = { \
        .ssname=#sname, \
        .ttname=#tname, \
        .run = __FNAME(sname, tname), \
        .skip = _skip, \
        .data = __data, \
        .setup = (SetupFunc)__setup, \
        .teardown = (TearDownFunc)__teardown, \
        .magic = CTEST_MAGIC };

#define CTEST_SETUP(sname) \
    void __attribute__ ((weak)) sname##_setup(struct sname##_data* data)

#define CTEST_TEARDOWN(sname) \
    void __attribute__ ((weak)) sname##_teardown(struct sname##_data* data)

#define __CTEST_INT(sname, tname, _skip) \
    void __FNAME(sname, tname)(); \
    __CTEST_STRUCT(sname, tname, _skip, 0, 0, 0) \
    void __FNAME(sname, tname)()

#define CTEST(sname, tname) __CTEST_INT(sname, tname, 0)
#define CTEST_SKIP(sname, tname) __CTEST_INT(sname, tname, 1)

#define __CTEST2_INT(sname, tname, _skip) \
    static struct sname##_data  __ctest_##sname##_data; \
    CTEST_SETUP(sname); \
    CTEST_TEARDOWN(sname); \
    void __FNAME(sname, tname)(struct sname##_data* data); \
    __CTEST_STRUCT(sname, tname, _skip, &__ctest_##sname##_data, sname##_setup, sname##_teardown) \
    void __FNAME(sname, tname)(struct sname##_data* data)

#define CTEST2(sname, tname) __CTEST2_INT(sname, tname, 0)
#define CTEST2_SKIP(sname, tname) __CTEST2_INT(sname, tname, 1)

void assert_str(const char* exp, const char*  real, const char* caller, int line);
#define ASSERT_STR(exp, real) assert_str(exp, real, __FILE__, __LINE__)

void assert_equal(int exp, int real, const char* caller, int line);
#define ASSERT_EQUAL(exp, real) assert_equal(exp, real, __FILE__, __LINE__)

void assert_not_equal(int exp, int real, const char* caller, int line);
#define ASSERT_NOT_EQUAL(exp, real) assert_not_equal(exp, real, __FILE__, __LINE__)

void assert_null(void* real, const char* caller, int line);
#define ASSERT_NULL(real) assert_null(real, __FILE__, __LINE__)

void assert_not_null(void* real, const char* caller, int line);
#define ASSERT_NOT_NULL(real) assert_not_null(real, __FILE__, __LINE__)

void assert_true(int real, const char* caller, int line);
#define ASSERT_TRUE(real) assert_true(real, __FILE__, __LINE__)

void assert_false(int real, const char* caller, int line);
#define ASSERT_FALSE(real) assert_false(real, __FILE__, __LINE__)

void assert_fail(const char* caller, int line);
#define ASSERT_FAIL() assert_fail(__FILE__, __LINE__)

#ifdef CTEST_MAIN

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <setjmp.h>

int ctest_errorsize;
char* ctest_errormsg;
#define MSG_SIZE 4096
static char ctest_errorbuffer[MSG_SIZE];
static jmp_buf ctest_err;

typedef int (*runfunc2)(void*);

CTEST(suite, test) { }

#define ANSI_BLACK "\033[0;30m"
#define ANSI_RED "\033[0;31m"
#define ANSI_GREEN "\033[0;32m"
#define ANSI_YELLOW "\033[0;33m"
#define ANSI_BLUE "\033[0;34m"
#define ANSI_MAGENTA "\033[0;35m"
#define ANSI_CYAN "\033[0;36m"
#define ANSI_GREY "\033[0;37m"
#define ANSI_DARKGREY "\033[01;30m"
#define ANSI_BRED "\033[01;31m"
#define ANSI_BGREEN "\033[01;32m"
#define ANSI_BYELLOW "\033[01;33m"
#define ANSI_BBLUE "\033[01;34m"
#define ANSI_BMAGENTA "\033[01;35m"
#define ANSI_BCYAN "\033[01;36m"
#define ANSI_WHITE "\033[01;37m"
#define ANSI_NORMAL "\033[0m"

typedef int (*filter_func)(struct ctest*);

static const char* suite_name = 0;

void CTEST_LOG(char *fmt, ...)
{
    va_list argp;
    int size = 0;
    size = snprintf(ctest_errormsg, ctest_errorsize, "  "ANSI_BLUE"LOG: ");
    ctest_errorsize -= size;
    ctest_errormsg += size;
    va_start(argp, fmt);
    size = vsnprintf(ctest_errormsg, ctest_errorsize, fmt, argp);
    ctest_errorsize -= size;
    ctest_errormsg += size;
    va_end(argp);
    size = snprintf(ctest_errormsg, ctest_errorsize, ANSI_NORMAL"\n");
    ctest_errorsize -= size;
    ctest_errormsg += size;
}

void CTEST_ERR(char *fmt, ...)
{
    va_list argp;
    int size = 0;
    size = snprintf(ctest_errormsg, ctest_errorsize, "  "ANSI_YELLOW"ERR: ");
    ctest_errorsize -= size;
    ctest_errormsg += size;
    va_start(argp, fmt);
    size = vsnprintf(ctest_errormsg, ctest_errorsize, fmt, argp);
    ctest_errorsize -= size;
    ctest_errormsg += size;
    va_end(argp);
    size = snprintf(ctest_errormsg, ctest_errorsize, ANSI_NORMAL"\n");
    ctest_errorsize -= size;
    ctest_errormsg += size;
}

void assert_str(const char* exp, const char*  real, const char* caller, int line) {
    if (strcmp(exp, real) != 0) {
        CTEST_ERR("%s:%d  expected '%s', got '%s'", caller, line, exp, real);
        longjmp(ctest_err, 1);
    }
}

void assert_equal(int exp, int real, const char* caller, int line) {
    if (exp != real) {
        CTEST_ERR("%s:%d  expected %d, got %d", caller, line, exp, real);
        longjmp(ctest_err, 1);
    }
}

void assert_not_equal(int exp, int real, const char* caller, int line) {
    if ((exp) == (real)) {
        CTEST_ERR("%s:%d  should not be %d", caller, line, real);
        longjmp(ctest_err, 1);
    }
}

void assert_null(void* real, const char* caller, int line) {
    if ((real) != NULL) {
        CTEST_ERR("%s:%d  should be NULL", caller, line);
        longjmp(ctest_err, 1);
    }
}

void assert_not_null(void* real, const char* caller, int line) {
    if (real == NULL) {
        CTEST_ERR("%s:%d  should not be NULL", caller, line);
        longjmp(ctest_err, 1);
    }
}

void assert_true(int real, const char* caller, int line) {
    if ((real) == 0) {
        CTEST_ERR("%s:%d  should be true", caller, line);
        longjmp(ctest_err, 1);
    }
}

void assert_false(int real, const char* caller, int line) {
    if ((real) != 0) {
        CTEST_ERR("%s:%d  should be false", caller, line);
        longjmp(ctest_err, 1);
    }
}

void assert_fail(const char* caller, int line) {
    CTEST_ERR("%s:%d  shouldn't come here", caller, line);
    longjmp(ctest_err, 1);
}


static int no_filter(struct ctest* t) { 
    return 1;
}

static int suite_filter(struct ctest* t) { 
    return strncmp(suite_name, t->ssname, strlen(suite_name)) == 0;
}

static u_int64_t getCurrentTime() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    u_int64_t now64 = now.tv_sec;
    now64 *= 1000000;
    now64 += (now.tv_nsec/1000);
    return now64;
}

int main(int argc, const char *argv[])
{
    volatile int total = 0;
    volatile int num_ok = 0;
    volatile int num_fail = 0;
    volatile int num_skip = 0;
    volatile int index = 1;
    volatile filter_func filter = no_filter;

    if (argc == 2) {
        suite_name = argv[1];
        filter = suite_filter;
    }

    u_int64_t t1 = getCurrentTime();

    struct ctest* ctest_begin = &__TNAME(suite, test);
    struct ctest* ctest_end = &__TNAME(suite, test);
    // find begin and end of section by comparing magics
    while (1) {
        struct ctest* t = ctest_begin-1;
        if (t->magic != CTEST_MAGIC) break;
        ctest_begin--;
    }
    while (1) {
        struct ctest* t = ctest_end+1;
        if (t->magic != CTEST_MAGIC) break;
        ctest_end++;
    }
    ctest_end++;    // end after last one

    struct ctest* test;    
    for (test = ctest_begin; test != ctest_end; test++) {
        if (test == &__ctest_suite_test) continue;
        if (filter(test)) total++;
    }

    for (test = ctest_begin; test != ctest_end; test++) {
        if (test == &__ctest_suite_test) continue;
        if (filter(test)) {
            ctest_errorbuffer[0] = 0;
            ctest_errorsize = MSG_SIZE-1;
            ctest_errormsg = ctest_errorbuffer;
            printf("TEST %d/%d %s:%s ", index, total, test->ssname, test->ttname);
            fflush(stdout);
            if (test->skip) {
                printf(ANSI_BYELLOW"[SKIPPED]"ANSI_NORMAL"\n");
                num_skip++;
            } else {
                if (test->setup) test->setup(test->data);
                int result = setjmp(ctest_err);
                if (result == 0) {
                    if (test->data) {
                        runfunc2 f2 = (runfunc2)test->run;
                        f2(test->data);
                    } else {
                        test->run();
                    }
                    // if we got here it's ok
                    printf("[OK]\n");
                    num_ok++;
                } else {
                    printf(ANSI_BRED"[FAIL]"ANSI_NORMAL"\n");
                    num_fail++;
                }
                if (test->teardown) test->teardown(test->data);
                if (ctest_errorsize != MSG_SIZE-1) printf("%s", ctest_errorbuffer);
            }
            index++;
        }
    }
    u_int64_t t2 = getCurrentTime();

    const char* color = (num_fail) ? ANSI_BRED : ANSI_GREEN;
    printf("%sRESULTS: %d tests,  %d ok, %d failed, %d skipped"ANSI_NORMAL"\n", color, total, num_ok, num_fail, num_skip);
    printf("Elapsed time: %lld ms\n", (t2 - t1) / 1000);
    return num_fail;
}

#endif

#endif

