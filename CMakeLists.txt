cmake_minimum_required(VERSION 3.24.2)

project(mini_lisp)

option(USE_GTEST "Use gtest" ON)

aux_source_directory(src SOURCES)
add_executable(mini_lisp ${SOURCES})
set_target_properties(
        mini_lisp
        PROPERTIES CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)
if (MSVC)
    target_compile_options(mini_lisp PRIVATE /utf-8 /Zc:preprocessor)
endif ()

if (USE_GTEST)
    find_package(GTest REQUIRED)
    enable_testing()

    aux_source_directory(tests TEST_SOURCES)
    list(APPEND TEST_SOURCES ${SOURCES})
    # remove main.cpp
    list(REMOVE_ITEM TEST_SOURCES src/main.cpp)
    message(TEST_SOURCES: ${TEST_SOURCES})
    add_executable(mini_lisp_tests ${TEST_SOURCES})

    target_include_directories(mini_lisp_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(mini_lisp_tests GTest::GTest GTest::Main)
    set_target_properties(
            mini_lisp_tests
            PROPERTIES CXX_STANDARD 23
            CXX_STANDARD_REQUIRED ON)

    gtest_discover_tests(mini_lisp_tests)
endif ()


