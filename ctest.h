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

typedef void (*SetupFunc)(void*);
typedef void (*TearDownFunc)(void*);
typedef void (*RunWithDataFunc)(void*);

struct ctest {
    const char* ssname;  // suite name
    const char* ttname;  // test name
    void (*run)();
    int skip;

    void* data;
    SetupFunc setup;
    TearDownFunc teardown;

    struct ctest *next;

    unsigned int magic;
};

#define __FNAME(sname, tname) __ctest_##sname##_##tname##_run
#define __TNAME(sname, tname) __ctest_##sname##_##tname
#define __PNAME(sname, tname) __ctest_##sname##_##tname##_pointer

#ifdef __APPLE__
#define __CTEST_APPLE
#endif

#if defined(_WIN32) && defined(_MSC_VER)
#define __CTEST_MSVC
#endif

//config for MSVC compiler
#ifdef __CTEST_MSVC

#define __CTEST_NO_TIME

#ifndef CTEST_ADD_TESTS_MANUALLY
//#define CTEST_ADD_TESTS_MANUALLY
#pragma section(".ctest$a")
#pragma section(".ctest$u")
#pragma section(".ctest$z")
#endif
//clear this flag for msvc
#ifdef CTEST_SEGFAULT
#undef CTEST_SEGFAULT
#endif
#define snprintf _snprintf_s
#endif

#ifdef CTEST_NO_JMP
#define __CTEST_NO_JMP
#endif

#ifdef CTEST_NO_TTY
#define __CTEST_NO_TTY
#endif

#define __CTEST_MAGIC (0xdeadbeef)

#ifdef CTEST_ADD_TESTS_MANUALLY
# define __Test_Section
#else
#ifdef __CTEST_APPLE
#define __Test_Section __attribute__ ((unused,section ("__DATA, .ctest")))
#elif defined (__CTEST_MSVC)
#define __Test_Section __declspec( allocate(".ctest$u"))
#else
#define __Test_Section __attribute__ ((unused,section (".ctest")))
#endif
#endif

#ifndef __CTEST_MSVC
#define __CTEST_STRUCT(sname, tname, _skip, __data, __setup, __teardown) \
    struct ctest __TNAME(sname, tname)  = { \
        .ssname=#sname, \
        .ttname=#tname, \
        .run = __FNAME(sname, tname), \
        .skip = _skip, \
        .data = __data, \
        .setup = (SetupFunc)__setup,					\
        .teardown = (TearDownFunc)__teardown,				\
        .next =  NULL, \
        .magic = __CTEST_MAGIC}; \
    void * __PNAME(sname, tname)[2] __Test_Section = {(void*)& __TNAME(sname,tname), (void*)__CTEST_MAGIC};
#else
//for msvc
#define __CTEST_STRUCT(sname, tname, _skip, __data, __setup, __teardown) \
    struct ctest __TNAME(sname, tname) = { \
         #sname, \
	 #tname, \
	 __FNAME(sname, tname), \
	 _skip, \
	 __data, \
	 (SetupFunc)__setup, \
         (TearDownFunc)__teardown, \
	 NULL, \
         __CTEST_MAGIC}; \
    __Test_Section void * __PNAME(sname, tname)[2]= {(void*)& __TNAME(sname,tname), (void *)__CTEST_MAGIC}; 
#endif

#define CTEST_DATA(sname) struct sname##_data

#ifndef __CTEST_MSVC
#define CTEST_SETUP(sname) \
    void __attribute__ ((weak)) sname##_setup(struct sname##_data* data)

#define CTEST_TEARDOWN(sname) \
    void __attribute__ ((weak)) sname##_teardown(struct sname##_data* data)
#else //for msvc
#define CTEST_SETUP(sname) \
  void sname##_setup(struct sname##_data* data)

#define CTEST_TEARDOWN(sname) \
  void sname##_teardown(struct sname##_data* data)
#endif

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
    void __FNAME(sname, tname)(struct sname##_data* data); \
    __CTEST_STRUCT(sname, tname, _skip, &__ctest_##sname##_data, SETUP_FNAME(sname), TEARDOWN_FNAME(sname)) \
    void __FNAME(sname, tname)(struct sname##_data* data)


void CTEST_LOG(char *fmt, ...);
void CTEST_ERR(char *fmt, ...);  // doesn't return

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

#define CTEST_ADD2(sname, tname) do { \
    extern struct ctest __TNAME(sname, tname); \
    __ctest_addTest(&__TNAME(sname, tname)); \
} while (0)

#endif // CTEST_ADD_TESTS_MANUALLY

void assert_str(const char* exp, const char* real, const char* caller, int line);
#define ASSERT_STR(exp, real) assert_str(exp, real, __FILE__, __LINE__)

void assert_data(const unsigned char* exp, int expsize,
                 const unsigned char* real, int realsize,
                 const char* caller, int line);
#define ASSERT_DATA(exp, expsize, real, realsize) \
    assert_data(exp, expsize, real, realsize, __FILE__, __LINE__)

void assert_equal(long exp, long real, const char* caller, int line);
#define ASSERT_EQUAL(exp, real) assert_equal(exp, real, __FILE__, __LINE__)

void assert_not_equal(long exp, long real, const char* caller, int line);
#define ASSERT_NOT_EQUAL(exp, real) assert_not_equal(exp, real, __FILE__, __LINE__)

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


#ifdef CTEST_MAIN

#ifndef __CTEST_NO_JMP
#include <setjmp.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifndef __CTEST_NO_TIME
#include <sys/time.h>
#include <inttypes.h>
#endif
#include <stdint.h>


#ifndef __CTEST_NO_TTY
#ifdef __CTEST_MSVC
#include <io.h>
#else
#include <unistd.h>
#endif
#endif

#include <stdlib.h>

#ifdef __CTEST_APPLE
#include <dlfcn.h>
#endif

//#define COLOR_OK

static size_t ctest_errorsize;
static char* ctest_errormsg;
#define MSG_SIZE 4096
static char ctest_errorbuffer[MSG_SIZE];
static __CTEST_JMPBUF ctest_err;
static int color_output = 1;
static const char* suite_name;
static const char* test_name;

typedef int (*filter_func)(struct ctest*);

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

#ifdef __CTEST_MSVC
#ifndef CTEST_ADD_TESTS_MANUALLY
__declspec(allocate(".ctest$a")) struct ctest * ctest_win_begin;
__declspec(allocate(".ctest$z")) struct ctest * ctest_win_end;
#endif
#endif

static CTEST(suite, test) { }


#define __CTEST_POINTER_NEXT(_test) (struct ctest **)((struct ctest **)(_test) + 2)
#define __CTEST_POINTER_PREV(_test) (struct ctest **)((struct ctest **)(_test) - 2)

/* First element of test list.
 */
static struct ctest * * __ctest_head_p = (struct ctest **)__PNAME(suite, test);

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

#ifndef __CTEST_MSVC
/* Add all tests to linked list automatically.
 */
static void __ctest_linkTests()
{
    struct ctest ** test;
    struct ctest ** ctest_begin = (struct ctest **)__PNAME(suite, test);
    struct ctest ** ctest_end = (struct ctest **)__PNAME(suite, test);

    // find begin and end of section by comparing magics
    while (1) {
        struct ctest** t = __CTEST_POINTER_PREV(ctest_begin);
	if (t[0] == NULL) break;
        if (t[1] != (struct ctest*)__CTEST_MAGIC) break;
        ctest_begin = t;
    }
    while (1) {
        struct ctest** t = __CTEST_POINTER_NEXT(ctest_end);
       	if (t[0] == NULL) break;
        if (t[1] != (struct ctest*)__CTEST_MAGIC) break;
        ctest_end = t;
    }
    ctest_end = __CTEST_POINTER_NEXT(ctest_end); // end after last one

    for (test = ctest_begin; test != ctest_end; test = __CTEST_POINTER_NEXT(test)) {
        struct ctest ** next_p = __CTEST_POINTER_NEXT(test);
	struct ctest * next;
        if (next_p == ctest_end)
            next = NULL;
	else
	    next = next_p[0];

        (*test)->next = next;
    }

    __ctest_head_p = ctest_begin;
}
#else //for msvc
static void __ctest_linkTests()
{
    struct ctest ** ctest_start = __ctest_head_p;
    struct ctest ** test;
    struct ctest * cur=ctest_start[0];

    for(test=&ctest_win_begin; test!=&ctest_win_end; test++){
      //check
      if(test[1] == (struct ctest*)__CTEST_MAGIC){
	//skip the start
	if((test[0]) == ctest_start[0]) continue;
	
	cur->next = test[0];
	cur=cur->next;
	cur->next=NULL;
      }
    }
}
#endif
#endif

static void msg_start(const char* color, const char* title) {
    int size;
    if (color_output) {
#ifndef __CTEST_MSVC
        size = snprintf(ctest_errormsg, ctest_errorsize, "%s", color);
#else
	size = _snprintf_s(ctest_errormsg, ctest_errorsize, _TRUNCATE, "%s", color);
#endif
        ctest_errorsize -= size;
        ctest_errormsg += size;
    }
#ifndef __CTEST_MSVC
    size = snprintf(ctest_errormsg, ctest_errorsize, "  %s: ", title);
#else
    size = _snprintf_s(ctest_errormsg, ctest_errorsize, _TRUNCATE, "  %s: ", title);
#endif
    ctest_errorsize -= size;
    ctest_errormsg += size;
}

static void msg_end() {
    int size;
    if (color_output) {
#ifndef __CTEST_MSVC
        size = snprintf(ctest_errormsg, ctest_errorsize, ANSI_NORMAL);
#else
	size = snprintf(ctest_errormsg, ctest_errorsize, _TRUNCATE, ANSI_NORMAL);
#endif
        ctest_errorsize -= size;
        ctest_errormsg += size;
    }
#ifndef __CTEST_MSVC
    size = snprintf(ctest_errormsg, ctest_errorsize, "\n");
#else
    size = snprintf(ctest_errormsg, ctest_errorsize, _TRUNCATE, "\n");
#endif
    ctest_errorsize -= size;
    ctest_errormsg += size;
}

void CTEST_LOG(char *fmt, ...)
{
    va_list argp;
    int size;
    msg_start(ANSI_BLUE, "LOG");

    va_start(argp, fmt);
    size = vsnprintf(ctest_errormsg, ctest_errorsize, fmt, argp);
    ctest_errorsize -= size;
    ctest_errormsg += size;
    va_end(argp);

    msg_end();
}

void CTEST_ERR(char *fmt, ...)
{
    va_list argp;
    int size;
    msg_start(ANSI_YELLOW, "ERR");

    va_start(argp, fmt);
    size = vsnprintf(ctest_errormsg, ctest_errorsize, fmt, argp);
    ctest_errorsize -= size;
    ctest_errormsg += size;
    va_end(argp);

    msg_end();
    __CTEST_LONGJMP(ctest_err, 1);
}

void assert_str(const char* exp, const char*  real, const char* caller, int line) {
    if ((exp == NULL && real != NULL) ||
        (exp != NULL && real == NULL) ||
        (exp && real && strcmp(exp, real) != 0)) {
        CTEST_ERR("%s:%d  expected '%s', got '%s'", caller, line, exp, real);
    }
}

void assert_data(const unsigned char* exp, int expsize,
                 const unsigned char* real, int realsize,
                 const char* caller, int line) {
    int i;
    if (expsize != realsize) {
        CTEST_ERR("%s:%d  expected %d bytes, got %d", caller, line, expsize, realsize);
    }
    for (i=0; i<expsize; i++) {
        if (exp[i] != real[i]) {
            CTEST_ERR("%s:%d expected 0x%02x at offset %d got 0x%02x",
                caller, line, exp[i], i, real[i]);
        }
    }
}

void assert_equal(long exp, long real, const char* caller, int line) {
    if (exp != real) {
        CTEST_ERR("%s:%d  expected %ld, got %ld", caller, line, exp, real);
    }
}

void assert_not_equal(long exp, long real, const char* caller, int line) {
    if ((exp) == (real)) {
        CTEST_ERR("%s:%d  should not be %ld", caller, line, real);
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
    return 1;
}

static int suite_filter(struct ctest* t) {
    return strncmp(suite_name, t->ssname, strlen(suite_name)) == 0;
}

static int suite_test_filter(struct ctest* t) {
  int suit_match, test_match;
  suit_match=(strncmp(suite_name, t->ssname, strlen(suite_name)) == 0);
  test_match=(strncmp(test_name, t->ttname, strlen(test_name)) == 0);
  return (suit_match & test_match);
}


#ifndef __CTEST_NO_TIME
static uint64_t getCurrentTime() {
    struct timeval now;
    gettimeofday(&now, NULL);
    uint64_t now64 = now.tv_sec;
    now64 *= 1000000;
    now64 += (now.tv_usec);
    return now64;
}
#endif

static void color_print(const char* color, const char* text) {
    if (color_output)
        printf("%s%s"ANSI_NORMAL"\n", color, text);
    else
        printf("%s\n", text);
}

#ifdef __CTEST_APPLE
static void *find_symbol(struct ctest *test, const char *fname)
{
    size_t len = strlen(test->ssname) + 1 + strlen(fname);
    char *symbol_name = (char *) malloc(len + 1);
    memset(symbol_name, 0, len + 1);
    snprintf(symbol_name, len + 1, "%s_%s", test->ssname, fname);

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

#ifdef CTEST_SEGFAULT
#include <signal.h>
static void sighandler(int signum)
{
    char msg[128];
    sprintf(msg, "[SIGNAL %d: %s]", signum, sys_siglist[signum]);
    color_print(ANSI_BRED, msg);
    fflush(stdout);

    /* "Unregister" the signal handler and send the signal back to the process
     * so it can terminate as expected */
    signal(signum, SIG_DFL);
    kill(getpid(), signum);
}
#endif

int ctest_main(int argc, const char *argv[])
{
    static int total = 0;
    static int num_ok = 0;
    static int num_fail = 0;
    static int num_skip = 0;
    static int index = 1;
    static filter_func filter = suite_all;

    const char* color = (num_fail) ? ANSI_BRED : ANSI_GREEN;
    char results[80];
    static struct ctest* test;

#ifdef CTEST_SEGFAULT
    signal(SIGSEGV, sighandler);
#endif

    if (argc == 2) {
        suite_name = argv[1];
        filter = suite_filter;
    }else if (argc == 3) {
        suite_name = argv[1];
	test_name = argv[2];
        filter = suite_test_filter;
    }

#ifndef __CTEST_NO_TTY
    color_output = isatty(1);
#endif

#ifdef __CTEST_MSVC
    color_output = 0;
#endif

#ifndef __CTEST_NO_TIME
    uint64_t t1 = getCurrentTime();
#endif
    /*
    struct ctest* ctest_begin = &__TNAME(suite, test);
    struct ctest* ctest_end = &__TNAME(suite, test);


    // find begin and end of section by comparing magics
    while (1) {
        struct ctest* t = ctest_begin-1;
        if (t->magic != __CTEST_MAGIC) break;
        ctest_begin--;
    }
    while (1) {
        struct ctest* t = ctest_end+1;
        if (t->magic != __CTEST_MAGIC) break;
        ctest_end++;
    }
    ctest_end++;    // end after last one
    */


#ifndef CTEST_ADD_TESTS_MANUALLY
    __ctest_linkTests();
#endif


    for (test = *(__ctest_head_p); test != NULL; test=test->next) {
        if (test == &__ctest_suite_test) continue;
        if (filter(test)) total++;
    }

    for (test = *(__ctest_head_p); test != NULL; test=test->next) {
        if (test == &__ctest_suite_test) continue;
        if (filter(test)) {
            ctest_errorbuffer[0] = 0;
            ctest_errorsize = MSG_SIZE-1;
            ctest_errormsg = ctest_errorbuffer;
            printf("TEST %d/%d %s:%s ", index, total, test->ssname, test->ttname);
            fflush(stdout);
            if (test->skip) {
                color_print(ANSI_BYELLOW, "[SKIPPED]");
                num_skip++;
            } else {
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

                    if (test->setup) test->setup(test->data);
                    if (test->data)
                      ((RunWithDataFunc)test->run)(test->data);
                    else
                      test->run();
                    if (test->teardown) test->teardown(test->data);
                    // if we got here it's ok
#ifdef COLOR_OK
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
            index++;
        }
    }
#ifndef __CTEST_NO_TIME
    uint64_t t2 = getCurrentTime();
#endif

#ifndef __CTEST_NO_TIME
    sprintf(results, "RESULTS: %d tests (%d ok, %d failed, %d skipped) ran in %"PRIu64" ms", total, num_ok, num_fail, num_skip, (t2 - t1)/1000);
#else
    sprintf(results, "RESULTS: %d tests (%d ok, %d failed, %d skipped)", total, num_ok, num_fail, num_skip);
#endif
    color_print(color, results);
    return num_fail;
}

#endif

#endif

