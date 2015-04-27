cmake_minimum_required(VERSION 2.8)

project(ctest C)

if(NOT MSVC)
add_executable( mytests
                main.c
		mytests.c
		)
else()
add_executable( mytests_vs
                main.c
                mytests_vs1.c
		mytests_vs2.c
		mytests_vs3.c
		)
endif()