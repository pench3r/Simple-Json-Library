cmake_minimum_required (VERSION 2.6)
project (simplejson_test C)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -ansi -pedantic -Wall -std=c99")
endif()

include_directories(include)
add_library(simplejson src/simplejson.c)
add_executable(simplejson_test src/sjtest.c)
target_link_libraries(simplejson_test simplejson)
