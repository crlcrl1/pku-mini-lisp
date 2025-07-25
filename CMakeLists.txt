cmake_minimum_required(VERSION 3.24)

project(mini_lisp)

option(USE_GTEST "Use GTest" ON)
option(USE_SANITIZER "Use sanitizer" OFF)
option(BUILD_CPP_EXAMPLES "Build C++ extension examples" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

aux_source_directory(src SOURCES)

add_executable(mini_lisp ${SOURCES})
set_target_properties(
        mini_lisp
        PROPERTIES CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON)
if (MSVC)
    target_compile_options(mini_lisp PRIVATE /utf-8 /Zc:preprocessor)
endif ()

if (NOT MSVC)
    target_link_options(mini_lisp PRIVATE -rdynamic)
endif ()

if (USE_SANITIZER)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(mini_lisp PRIVATE -fsanitize=address -fsanitize=undefined)
        target_link_options(mini_lisp PRIVATE -fsanitize=address -fsanitize=undefined)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(mini_lisp PRIVATE -fsanitize=address -fsanitize=undefined)
        target_link_options(mini_lisp PRIVATE -fsanitize=address -fsanitize=undefined)
    else ()
        message(WARNING "Sanitizer is not supported by the compiler")
    endif ()
endif ()

set(REPLXX_BUILD_EXAMPLES OFF)
set(CMAKE_CXX_STANDARD 23)
add_subdirectory(third_party/replxx)

target_link_libraries(mini_lisp PRIVATE replxx::replxx)

if (USE_GTEST)
    find_package(GTest REQUIRED)
    enable_testing()

    aux_source_directory(tests TEST_SOURCES)
    list(APPEND TEST_SOURCES ${SOURCES})
    # remove main.cpp
    list(REMOVE_ITEM TEST_SOURCES src/main.cpp)
    add_executable(mini_lisp_tests ${TEST_SOURCES})

    target_include_directories(mini_lisp_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(mini_lisp_tests PRIVATE GTest::GTest GTest::Main)
    target_link_libraries(mini_lisp_tests PRIVATE replxx::replxx)
    set_target_properties(
            mini_lisp_tests
            PROPERTIES CXX_STANDARD 23
            CXX_STANDARD_REQUIRED ON)

    gtest_discover_tests(mini_lisp_tests)
endif ()

if (BUILD_CPP_EXAMPLES)
    add_subdirectory(examples/cpp_extension)
endif ()
