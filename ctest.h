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

// for NULL
#include <stddef.h>

// detecting GCC
#ifdef __GNUC__
# define __CTEST_GCC
#endif

// detecting Apple
#ifdef __APPLE__
# define __CTEST_APPLE
#endif

// detecting MikroC compiler
#if defined(__MIKROC_PRO_FOR_PIC32__) ||\
    defined(__MIKROC_PRO_FOR_PIC__)   ||\
    defined(__MIKROC_PRO_FOR_DSPIC__) ||\
    defined(__MIKROC_PRO_FOR_AVR__)   ||\
    defined(__MIKROC_PRO_FOR_ARM__)         // Only MikroC for ARM was really tested
# define __CTEST_MIKROC
#endif

#ifdef __CTEST_MIKROC /** MikroC-specific **/

/* Linker sections are not supported, so user should add tests manually.
 */
# ifndef CTEST_ADD_TESTS_MANUALLY
#  define CTEST_ADD_TESTS_MANUALLY
# endif

/* Enabling workaround: 'data' is reserved keyword, we'll use _data instead.
 */
# ifndef CTEST_DATA_IS_KEYWORD
#  define CTEST_DATA_IS_KEYWORD
# endif

/* Enabling workaround: function cast doesn't work in structure initializers
 */
# define __CTEST_NO_FUNC_CAST

/* isatty() is not available.
 */
# define __CTEST_NO_TTY

/* longjmp() is not available in MikroC, at least for ARM.
 * It is probably available in MikroC for PIC, but that wasn't tested.
 */
# define __CTEST_NO_JMP

/* gettimeofday() is not available.
 */
# define __CTEST_NO_TIME

/* printf() and fflush() are not available().
 */
# define __CTEST_NO_PRINTF

/* snprintf() is not available.
 */
# define __CTEST_NO_SNPRINTF

/* `weak' attribute is not available.
 */
# define __CTEST_NO_WEAK

#endif // __CTEST_MIKROC

/** Check user macros **/

#ifdef __CTEST_NO_TIME
# ifndef CTEST_USER_TIME
#  error It seems that gettimeofday() is not available, please define CTEST_USER_TIME() macro that will \
         return current time in milliseconds
# endif
#endif // __CTEST_NO_TIME

#ifdef __CTEST_NO_PRINTF
# ifndef CTEST_USER_PRINTF
#  error It seems that printf() is not available, please define CTEST_USER_PRINTF() macro that will \
         emulate it
# endif
# ifndef CTEST_USER_FLUSH
#  error It seems that fflush() is not available, please define CTEST_USER_FLUSH() macro that will \
         emulate fflush(stdout)
# endif
#endif // __CTEST_NO_PRINTF

/** Configure internal macros **/

#ifdef CTEST_USER_TIME
# define __CTEST_TIME CTEST_USER_TIME
#else
# define __CTEST_TIME ctest_getCurrentTime
#endif // CTEST_USER_TIME

#ifdef CTEST_USER_PRINTF
# define __CTEST_PRINTF CTEST_USER_PRINTF
#else
# define __CTEST_PRINTF printf
#endif // CTEST_USER_PRINTF

#ifdef CTEST_USER_FLUSH
# define __CTEST_FLUSH CTEST_USER_FLUSH
#else
# define __CTEST_FLUSH() fflush(stdout)
#endif // CTEST_USER_FLUSH

#ifdef CTEST_DATA_IS_KEYWORD
# define __CTEST_DATA _data
#else
# define __CTEST_DATA data
#endif // CTEST_DATA_IS_KEYWORD

#ifdef __CTEST_NO_FUNC_CAST
# define __CTEST_FUNC_CAST(_type, _name) (_name)
#else
# define __CTEST_FUNC_CAST(_type, _name) (_type)(_name)
#endif // __CTEST_NO_CAST

#ifdef __CTEST_NO_SNPRINTF
/* If snprintf() is not available, use sprintf() and ignore buffer size.
 * It is unsafe and can cause buffer overflow!
 * Maybe there is some better solution..
 */
# define __CTEST_SNPRINTF(_buf, _len, ...) sprintf(_buf, __VA_ARGS__)
#else
# define __CTEST_SNPRINTF snprintf
#endif // __CTEST_NO_SNPRINTF

/* If longjmp() is not available, integer flag will be used instead of jmp_buf.
 *
 * __CTEST_SETJMP() will clear the flag and return zero, and __CTEST_LONGJMP()
 * will set the flag to its argument. __CTEST_ERROR_CODE() will return that flag.
 *
 * If longjmp() is available, jmp_buf will be used as usual and __CTEST_ERROR_CODE()
 * will always return zero.
 *
 * You can check both __CTEST_SETJMP() and __CTEST_ERROR_CODE() return value
 * to detect error in a portable way.
 */
#ifdef __CTEST_NO_JMP
# define __CTEST_JMPBUF                 int
# define __CTEST_ERROR_CODE(_var)       (_var)
# define __CTEST_SETJMP(_var)           (_var = 0)
# define __CTEST_LONGJMP(_var, _err)    (_var = _err)
#else // !__CTEST_NO_JMP
# define __CTEST_JMPBUF                 jmp_buf
# define __CTEST_ERROR_CODE(_var)       (0)
# define __CTEST_SETJMP(_var)           setjmp(_var)
# define __CTEST_LONGJMP(_var, _err)    longjmp(_var, _err)
#endif // __CTEST_NO_JMP

#ifdef __CTEST_NO_WEAK
# define __CTEST_WEAK
#else
# define __CTEST_WEAK __attribute__ ((weak))
#endif


typedef void (*SetupFunc)(void*);
typedef void (*TearDownFunc)(void*);
typedef void (*RunWithDataFunc)(void*);

struct ctest {
    const char* ssname;  // suite name
    const char* ttname;  // test name
    void (*run)();
    int skip;

    void* __CTEST_DATA;  // expands to '_data' if CTEST_DATA_IS_KEYWORD defined and to 'data' otherwise
    SetupFunc setup;
    TearDownFunc teardown;

    struct ctest *next;

    unsigned int magic;
};

#define __FNAME(sname, tname) __ctest_##sname##_##tname##_run
#define __TNAME(sname, tname) __ctest_##sname##_##tname

#define __CTEST_MAGIC (0xdeadbeef)

#if defined(_WIN64) || defined(__amd64__)
# define __CTEST_ALIGN 128
#else
# define __CTEST_ALIGN 64
#endif

/* Since ctest struct can be aligned by compiler, we will align it explicitly and use
 * following macros to access next and previous aligned pointers.
 */
#define __CTEST_NEXT(_test) (struct ctest *)((char *)(_test) + __CTEST_ALIGN)
#define __CTEST_PREV(_test) (struct ctest *)((char *)(_test) - __CTEST_ALIGN)

#ifdef CTEST_ADD_TESTS_MANUALLY
# define __Test_Section
#else // !CTEST_ADD_TESTS_MANUALLY
# ifdef __CTEST_APPLE
#  define __Test_Section __attribute__ ((unused,section ("__DATA, .ctest"))) __attribute__ ((aligned (__CTEST_ALIGN)))
# else // !__CTEST_APPLE
#  define __Test_Section __attribute__ ((unused,section (".ctest"))) __attribute__ ((aligned (__CTEST_ALIGN)))
# endif // __CTEST_APPLE
#endif // CTEST_ADD_TESTS_MANUALLY

/* MikroC issues:
 *    - named initializers are not supported;
 *    - function cast in structure initializer doesn't work in some cases.
 */
#define __CTEST_STRUCT(sname, tname, _skip, __data, __setup, __teardown) \
    struct ctest __TNAME(sname, tname) __Test_Section = { \
        /* .ssname   = */ #sname, \
        /* .ttname   = */ #tname, \
        /* .run      = */ (void *)__FNAME(sname, tname), \
        /* .skip     = */ _skip, \
        /* .data     = */ __data, \
        /* .setup    = */ __CTEST_FUNC_CAST(SetupFunc, __setup), \
        /* .teardown = */ __CTEST_FUNC_CAST(TearDownFunc, __teardown), \
        /* .next     = */ NULL, \
        /* .magic    = */ __CTEST_MAGIC };

#define CTEST_DATA(sname) struct sname##_data

#define CTEST_SETUP(sname) \
    void __CTEST_WEAK sname##_setup(struct sname##_data* __CTEST_DATA)

#define CTEST_TEARDOWN(sname) \
    void __CTEST_WEAK sname##_teardown(struct sname##_data* __CTEST_DATA)

#define __CTEST_INTERNAL(sname, tname, _skip) \
    void __FNAME(sname, tname)(); \
    __CTEST_STRUCT(sname, tname, _skip, NULL, NULL, NULL) \
    void __FNAME(sname, tname)()

#ifdef __CTEST_APPLE
#define SETUP_FNAME(sname) NULL
#define TEARDOWN_FNAME(sname) NULL
#else
#define SETUP_FNAME(sname) sname##_setup
#define TEARDOWN_FNAME(sname) sname##_teardown
#endif

#define __CTEST2_INTERNAL(sname, tname, _skip) \
    static struct sname##_data  __ctest_##sname##_data; \
    CTEST_SETUP(sname); \
    CTEST_TEARDOWN(sname); \
    void __FNAME(sname, tname)(struct sname##_data* __CTEST_DATA); \
    __CTEST_STRUCT(sname, tname, _skip, &__ctest_##sname##_data, SETUP_FNAME(sname), TEARDOWN_FNAME(sname)) \
    void __FNAME(sname, tname)(struct sname##_data* __CTEST_DATA)


#define CTEST(sname, tname) __CTEST_INTERNAL(sname, tname, 0)
#define CTEST_SKIP(sname, tname) __CTEST_INTERNAL(sname, tname, 1)

#define CTEST2(sname, tname) __CTEST2_INTERNAL(sname, tname, 0)
#define CTEST2_SKIP(sname, tname) __CTEST2_INTERNAL(sname, tname, 1)


#ifdef CTEST_ADD_TESTS_MANUALLY

void __ctest_addTest(struct ctest *);

#define CTEST_ADD(sname, tname) do { \
    extern struct ctest __TNAME(sname, tname); \
    __ctest_addTest(&__TNAME(sname, tname)); \
} while (0)

/* MikroC needs this workaround becuase its compiler can remove setup, run and
 * teardown functions.
 *
 * Possible workaround is to declare non-static variable that holds function
 * address AND call that function explicitly (without pointer casts).
 *
 * This code is never executed, but it forces compiler not to remove that functions.
 * Sorry.
 */
#ifdef __CTEST_MIKROC
# define __CTEST_MIKROC_FUNCPTR_WORKAROUND(sname, tname) \
    if (0) { \
        void *setup = & SETUP_FNAME(sname); \
        void *run = & __FNAME(sname, tname); \
        void *teardown = & TEARDOWN_FNAME(sname); \
        SETUP_FNAME(sname)(NULL); \
        __FNAME(sname, tname)(NULL); \
        TEARDOWN_FNAME(sname)(NULL); \
    }
#else
# define __CTEST_MIKROC_FUNCPTR_WORKAROUND(sname, tname)
#endif

#define CTEST_ADD2(sname, tname) do { \
    extern struct ctest __TNAME(sname, tname); \
    __ctest_addTest(&__TNAME(sname, tname)); \
    __CTEST_MIKROC_FUNCPTR_WORKAROUND(sname, tname) \
} while (0)

#endif // CTEST_ADD_TESTS_MANUALLY


void assert_str(const char* exp, const char*  real, const char* caller, int line);
#define ASSERT_STR(exp, real) assert_str(exp, real, __FILE__, __LINE__)

void assert_equal(long exp, long real, const char* caller, int line);
#define ASSERT_EQUAL(exp, real) assert_equal(exp, real, __FILE__, __LINE__)

void assert_not_equal(long exp, long real, const char* caller, int line);
#define ASSERT_NOT_EQUAL(exp, real) assert_not_equal(exp, real, __FILE__, __LINE__)

void assert_null(void* real, const char* caller, int line);
#define ASSERT_NULL(real) assert_null((void*)real, __FILE__, __LINE__)

void assert_not_null(void* real, const char* caller, int line);
#define ASSERT_NOT_NULL(real) assert_not_null(real, __FILE__, __LINE__)

void assert_true(int real, const char* caller, int line);
#define ASSERT_TRUE(real) assert_true(real, __FILE__, __LINE__)

void assert_false(int real, const char* caller, int line);
#define ASSERT_FALSE(real) assert_false(real, __FILE__, __LINE__)

void assert_fail(const char* caller, int line);
#define ASSERT_FAIL() assert_fail(__FILE__, __LINE__)

extern size_t ctest_errorsize;
extern char* ctest_errormsg;
extern int ctest_gcc_style;

void ctest_msg_start(const char* color, const char* title);
void ctest_msg_end();

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

/* Print log message.
 * Using macro instead of function because vsnprintf() can be not available.
 */
#define CTEST_LOG(...)  do { \
    int _size; \
    ctest_msg_start(ANSI_BLUE, "LOG"); \
    _size = __CTEST_SNPRINTF(ctest_errormsg, ctest_errorsize, __VA_ARGS__); \
    ctest_errorsize -= _size; \
    ctest_errormsg += _size; \
    ctest_msg_end(); \
} while (0)

/* Print error message in default ctest style: `ERR <file>:<line> <error>'.
 * Using macro instead of function because vsnprintf() can be not available.
 */
#define CTEST_ERR_DEFAULT(_file, _line, ...) do { \
    int _size; \
    ctest_msg_start(ANSI_YELLOW, "ERR"); \
    _size = __CTEST_SNPRINTF(ctest_errormsg, ctest_errorsize, "%s:%d ", _file, _line); \
    ctest_errorsize -= _size; \
    ctest_errormsg += _size; \
    _size = __CTEST_SNPRINTF(ctest_errormsg, ctest_errorsize, __VA_ARGS__); \
    ctest_errorsize -= _size; \
    ctest_errormsg += _size; \
    ctest_msg_end(); \
} while (0)

/* Print error message in gcc style: `<file>:<line>: error: <error>'.
 *
 * This format is determined by many programs, including eclipse, emacs and colorgcc, so
 * test errors can be automatically highlighted during your project build.
 *
 * Using macro instead of function because vsnprintf() can be not available.
 */
#define CTEST_ERR_GCC_STYLE(_file, _line, ...) do { \
    int _size = __CTEST_SNPRINTF(ctest_errormsg, ctest_errorsize, "%s:%d: error: ", _file, _line); \
    ctest_errorsize -= _size; \
    ctest_errormsg += _size; \
    _size = __CTEST_SNPRINTF(ctest_errormsg, ctest_errorsize, __VA_ARGS__); \
    ctest_errorsize -= _size; \
    ctest_errormsg += _size; \
    _size = __CTEST_SNPRINTF(ctest_errormsg, ctest_errorsize, "%s", "\n"); \
    ctest_errorsize -= _size; \
    ctest_errormsg += _size; \
} while (0)

/* Print error message in ctest or gcc style, depending on global flag.
 */
#define CTEST_ERR(_file, _line, ...) do { \
    if (ctest_gcc_style) \
        CTEST_ERR_GCC_STYLE(_file, _line, __VA_ARGS__); \
    else \
        CTEST_ERR_DEFAULT(_file, _line, __VA_ARGS__); \
} while (0)

#ifndef CTEST_USER_PRINTF
#include <stdio.h>
#endif

#ifdef CTEST_MAIN

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#ifndef __CTEST_NO_JMP
#include <setjmp.h>
#endif

#ifndef __CTEST_NO_TTY
#include <unistd.h>
#endif

#ifndef CTEST_USER_TIME
#include <sys/time.h>
#include <stdint.h>
#endif

#ifdef __CTEST_APPLE
#include <dlfcn.h>
#endif

//#define CTEST_COLOR_OK

size_t ctest_errorsize;
char* ctest_errormsg;
int ctest_gcc_style;
#define MSG_SIZE 4096
static char ctest_errorbuffer[MSG_SIZE];
static __CTEST_JMPBUF ctest_err;
static int color_output = 1;
static const char* suite_name;

typedef int (*filter_func)(struct ctest*);

#ifndef __CTEST_MIKROC
// MikroC linker workaround
static
#endif
CTEST(suite, test) { }

/* First element of test list.
 */
static struct ctest *__ctest_head = &__TNAME(suite, test);

#ifdef CTEST_ADD_TESTS_MANUALLY

/* Last element of test list.
 */
static struct ctest *__ctest_tail = &__TNAME(suite, test);

/* Add test to linked list manually.
 */
void __ctest_addTest(struct ctest *test)
{
    __ctest_tail->next = test;
    __ctest_tail = test;
}

#else // !CTEST_ADD_TESTS_MANUALLY

/* Add all tests to linked list automatically.
 */
static void __ctest_linkTests()
{
    struct ctest *test;
    struct ctest* ctest_begin = &__TNAME(suite, test);
    struct ctest* ctest_end = &__TNAME(suite, test);

    // find begin and end of section by comparing magics
    while (1) {
        struct ctest* t = __CTEST_PREV(ctest_begin);
        if (t->magic != __CTEST_MAGIC) break;
        ctest_begin = t;
    }
    while (1) {
        struct ctest* t = __CTEST_NEXT(ctest_end);
        if (t->magic != __CTEST_MAGIC) break;
        ctest_end = t;
    }
    ctest_end = __CTEST_NEXT(ctest_end); // end after last one

    for (test = ctest_begin; test != ctest_end; test = __CTEST_NEXT(test)) {
        struct ctest *next = __CTEST_NEXT(test);
        if (next == ctest_end)
            next = NULL;

        test->next = next;
    }

    __ctest_head = ctest_begin;
}

#endif // CTEST_ADD_TESTS_MANUALLY


void ctest_msg_start(const char* color, const char* title) {
    int size;
    if (color_output) {
        size = __CTEST_SNPRINTF(ctest_errormsg, ctest_errorsize, "%s", color);
        ctest_errorsize -= size;
        ctest_errormsg += size;
    }
    size = __CTEST_SNPRINTF(ctest_errormsg, ctest_errorsize, "  %s: ", title);
    ctest_errorsize -= size;
    ctest_errormsg += size;
}

void ctest_msg_end() {
    int size;
    if (color_output) {
        size = __CTEST_SNPRINTF(ctest_errormsg, ctest_errorsize, ANSI_NORMAL);
        ctest_errorsize -= size;
        ctest_errormsg += size;
    }
    size = __CTEST_SNPRINTF(ctest_errormsg, ctest_errorsize, "\n");
    ctest_errorsize -= size;
    ctest_errormsg += size;
}

void assert_str(const char* exp, const char*  real, const char* caller, int line) {
    if ((exp == NULL && real != NULL) ||
        (exp != NULL && real == NULL) ||
        (exp && real && strcmp(exp, real) != 0)) {
        CTEST_ERR(caller, line, "expected '%s', got '%s'", exp, real);
        __CTEST_LONGJMP(ctest_err, 1);
    }
}

void assert_equal(long exp, long real, const char* caller, int line) {
    if (exp != real) {
        CTEST_ERR(caller, line, "expected %ld, got %ld", exp, real);
        __CTEST_LONGJMP(ctest_err, 1);
    }
}

void assert_not_equal(long exp, long real, const char* caller, int line) {
    if ((exp) == (real)) {
        CTEST_ERR(caller, line, "should not be %ld", real);
        __CTEST_LONGJMP(ctest_err, 1);
    }
}

void assert_null(void* real, const char* caller, int line) {
    if ((real) != NULL) {
        CTEST_ERR(caller, line, "should be NULL");
        __CTEST_LONGJMP(ctest_err, 1);
    }
}

void assert_not_null(void* real, const char* caller, int line) {
    if (real == NULL) {
        CTEST_ERR(caller, line, "should not be NULL");
        __CTEST_LONGJMP(ctest_err, 1);
    }
}

void assert_true(int real, const char* caller, int line) {
    if ((real) == 0) {
        CTEST_ERR(caller, line, "should be true");
        __CTEST_LONGJMP(ctest_err, 1);
    }
}

void assert_false(int real, const char* caller, int line) {
    if ((real) != 0) {
        CTEST_ERR(caller, line, "should be false");
        __CTEST_LONGJMP(ctest_err, 1);
    }
}

void assert_fail(const char* caller, int line) {
    CTEST_ERR(caller, line, "shouldn't come here");
    __CTEST_LONGJMP(ctest_err, 1);
}


static int suite_all(struct ctest* t) {
    return 1;
}

static int suite_filter(struct ctest* t) {
    return strncmp(suite_name, t->ssname, strlen(suite_name)) == 0;
}

#ifndef CTEST_USER_TIME
static uint64_t ctest_getCurrentTime() {
    struct timeval now;
    gettimeofday(&now, NULL);
    uint64_t now64 = now.tv_sec;
    now64 *= 1000000;
    now64 += (now.tv_usec);
    return now64;
}
#endif // CTEST_USER_TIME

static void color_print(const char* color, const char* text) {
    if (color_output)
        __CTEST_PRINTF("%s%s"ANSI_NORMAL"\n", color, text);
    else
        __CTEST_PRINTF("%s\n", text);
}

#ifdef __CTEST_APPLE
static void *find_symbol(struct ctest *test, const char *fname)
{
    size_t len = strlen(test->ssname) + 1 + strlen(fname);
    char *symbol_name = (char *) malloc(len + 1);
    memset(symbol_name, 0, len + 1);
    __CTEST_SNPRINTF(symbol_name, len + 1, "%s_%s", test->ssname, fname);

    //fprintf(stderr, ">>>> dlsym: loading %s\n", symbol_name);
    void *symbol = dlsym(RTLD_DEFAULT, symbol_name);
    if (!symbol) {
        //fprintf(stderr, ">>>> ERROR: %s\n", dlerror());
    }
    // returns NULL on error

    free(symbol_name);
    return symbol;
}
#endif


int ctest_main(int argc, const char *argv[])
{
    static int total;
    static int num_ok;
    static int num_fail;
    static int num_skip;
    static int index;
    static filter_func filter = suite_all;

    unsigned long long t1;
    unsigned long long t2;

    struct ctest* test;
    const char* color;
    static char results[80];

    int arg;

    // Reinitialize static variables to allow calling ctest_main() more than once.
    total = 0;
    num_ok = 0;
    num_fail = 0;
    num_skip = 0;
    index = 1;

#ifndef __CTEST_NO_TTY
    color_output = isatty(1);
#endif

    for (arg = 1; arg < argc; ++arg) {
        if (argv[arg][0] == '-') {
            if (strcmp(argv[arg], "--colored") == 0) {
                color_output = 1;
            }
            else if (strcmp(argv[arg], "--gcc-style") == 0) {
                ctest_gcc_style = 1;
            }
        }
        else {
            suite_name = argv[arg];
            filter = suite_filter;
        }
    }

    t1 = __CTEST_TIME();

#ifndef CTEST_ADD_TESTS_MANUALLY
    __ctest_linkTests();
#endif

    for (test = __ctest_head; test != NULL; test = test->next) {
        if (test == &__TNAME(suite, test)) continue;
        if (filter(test)) total++;
    }

    for (test = __ctest_head; test != NULL; test = test->next) {
        if (test == &__TNAME(suite, test)) continue;
        if (filter(test)) {
            ctest_errorbuffer[0] = 0;
            ctest_errorsize = MSG_SIZE-1;
            ctest_errormsg = ctest_errorbuffer;
            __CTEST_PRINTF("TEST %d/%d %s:%s ", index, total, test->ssname, test->ttname);
            __CTEST_FLUSH();
            if (test->skip) {
                color_print(ANSI_BYELLOW, "[SKIPPED]");
                num_skip++;
            } else {
                int error_code;
                int result = __CTEST_SETJMP(ctest_err);
                if (result == 0) {
#ifdef __CTEST_APPLE
                    if (!test->setup) {
                        test->setup = find_symbol(test, "setup");
                    }
                    if (!test->teardown) {
                        test->teardown = find_symbol(test, "teardown");
                    }
#endif

                    if (test->setup) test->setup(test->__CTEST_DATA);
                    if (test->__CTEST_DATA)
                      ((RunWithDataFunc)test->run)(test->__CTEST_DATA);
                    else
                      test->run();
                    if (test->teardown) test->teardown(test->__CTEST_DATA);
                    /* If longjmp() is available and we got here, it's ok and __CTEST_ERROR_CODE()
                     * will also return zero.
                     *
                     * If longjmp() is not available, __CTEST_ERROR_CODE() will return non-zero
                     * if error was detected.
                     */
                    error_code = __CTEST_ERROR_CODE(ctest_err);
                } else {
                    // If we got here, longjmp() is available and was called, so error occured.
                    error_code = 1;
                }
                if (error_code == 0) {
#ifdef CTEST_COLOR_OK
                    color_print(ANSI_BGREEN, "[OK]");
#else
                    __CTEST_PRINTF("[OK]\n");
#endif
                    num_ok++;
                } else {
                    color_print(ANSI_BRED, "[FAIL]");
                    num_fail++;
                }
                if (ctest_errorsize != MSG_SIZE-1) __CTEST_PRINTF("%s", ctest_errorbuffer);
            }
            index++;
        }
    }
    t2 = __CTEST_TIME();

    color = (num_fail) ? ANSI_BRED : ANSI_GREEN;
    __CTEST_SNPRINTF(results, sizeof(results),
                     "RESULTS: %d tests (%d ok, %d failed, %d skipped) ran in %lld ms",
                         total, num_ok, num_fail, num_skip, (t2 - t1)/1000);
    color_print(color, results);
    return num_fail;
}

#endif // CTEST_MAIN

#endif // CTEST_H
