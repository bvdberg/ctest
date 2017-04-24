cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
project(ctest)

if(CMAKE_VERSION VERSION_LESS "3.1")
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_C_FLAGS "-std=c99 ${CMAKE_C_FLAGS}")
    endif()
else()
    set(CMAKE_C_STANDARD 99)
endif()

enable_testing()

set(ctest_SOURCES
    ctest.h
    mytests.c
    main.c
)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --verbose)

add_executable(test0 ${ctest_SOURCES})
add_dependencies(check test0)
add_test(test0 test0)

add_executable(test1 ${ctest_SOURCES})
add_definitions(-DCTEST_SEGFAULT)
add_dependencies(check test1)
add_test(test1 test1)

add_executable(test2 ${ctest_SOURCES})
add_definitions(-DCTEST_NO_COLORS)
add_dependencies(check test2)
add_test(test2 test2)

add_executable(test3 ${ctest_SOURCES})
add_definitions(-DCTEST_COLOR_OK)
add_dependencies(check test3)
add_test(test3 test3)
