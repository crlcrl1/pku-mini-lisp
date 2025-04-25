cmake_minimum_required(VERSION 3.24)

project(mini_lisp_extension)

include_directories(../../src/api)

set(CMAKE_CXX_STANDARD 23)

add_library(fib SHARED fib.cpp)