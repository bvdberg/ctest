ctest
=====
ctest is a unit test framework for software written in C.

Features:
---------
- Adding tests with minimal hassle (no manual adding to suites or testlists!)
- Supports suites of tests
- Supports `setup()` / `teardown()` per test
- Output format not messed up when tests fail, so easy to parse.
- Displays elapsed time, so you can keep your tests fast
- Uses coloring for easy error recognition
- Only use coloring if output goes to terminal (not file/process)
- It's small (a little over 300 lines of code!)
- It's easy to integrate (only 1 header file)
- Has SKIP option to skip certain test (no commenting test out anymore)
- Linux + OS/X support

Possible improvements:
----------------------
- Try to use same macro for tests with/without setup/teardown. So no `CTEST2`
- Silent option that just shows final results (usefull when running often)
- Wiki page with examples

----------------------------------------------------------------------------

Using ctest:
------------
**Test example:**

    CTEST(suite, test1) {
        ASSERT_STR("foo", "foo");
    }

    CTEST(suite, test2) {
        ASSERT_EQUAL(1, 2);
    }

**No** further typing is needed! ctest does the rest.

**Example output when running ctest:**

    $ ./test
    TEST 1/2 suite1:test1 [OK]
    TEST 2/2 suite1:test2 [FAIL]
      ERR: mytests.c:4  expected 1, got 2
    RESULTS: 2 tests (1 ok, 1 failed, 0 skipped) ran in 1 ms

There can be one argument to: `./test <suite>`. For example: 

    $ ./test timer

will run all tests from suites starting with 'timer'

*NOTE: When piping output to a file/process, ctest will not color the output*

----------------------------------------------------------------------------

Fixtures:
---------
A testcase with a `setup()`/`teardown()` is described below. An `unsigned
char` buffer is `malloc`ed before each test in the suite and `free`d afterwards.

    CTEST_DATA(mytest) {
        unsigned char* buffer;
    };

*NOTE: The `mytest_data` struct is available in setup/teardown/run functions as 'data'*

    CTEST_SETUP(mytest) {
        data->buffer = (unsigned char*)malloc(1024);
    }

    CTEST_TEARDOWN(mytest) {
        free(data->buffer);
    }

*NOTE: `setup()` will be called before this test (and ony other test in the same suite)*  
*NOTE: `CTEST_LOG()` can be used to log warnings consistent with the normal output format*

    CTEST2(mytest, test1) {
        CTEST_LOG("%s()  data=%p  buffer=%p", __func__, data, data->buffer);
    }

*NOTE: `teardown()` will be called after the test completes*  
*NOTE: It's possible to only have a `setup()` or `teardown()`*

----------------------------------------------------------------------------

Skipping:
---------
Instead of commenting out a test (and subsequently never remembering to turn it
back on, ctest allows skipping of tests. Skipped tests are still shown when running
tests, but not run. To skip a test add `_SKIP`:

`CTEST_SKIP(..)` or `CTEST2_SKIP(..)`

----------------------------------------------------------------------------

License:
--------
Copyright 2011,2012 Bas van den Berg

Licensed under the Apache License, Version 2.0 (the "License");  
you may not use this file except in compliance with the License.  
You may obtain a copy of the License at  

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software  
distributed under the License is distributed on an "AS IS" BASIS,  
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
See the License for the specific language governing permissions and  
limitations under the License.  
