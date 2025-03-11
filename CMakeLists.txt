cmake_minimum_required(VERSION 3.24.2)

project(mini_lisp)

option(USE_GTEST "Use GTest" ON)
option(USE_SANITIZER "Use sanitizer" OFF)
option(USE_LLVM "Use LLVM" OFF)
option(DYNAMIC_LINK_LLVM "Link LLVM dynamically" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

aux_source_directory(src SOURCES)
if (USE_LLVM)
    file(GLOB LLVM_SOURCES src/llvm/*.cpp)
    list(APPEND SOURCES ${LLVM_SOURCES})
    find_package(LLVM REQUIRED CONFIG)
    llvm_map_components_to_libnames(llvm_libs
            Core
            ExecutionEngine
            Interpreter
            MC
            MCJIT
            Support
            nativecodegen
    )

endif ()

add_executable(mini_lisp ${SOURCES})
set_target_properties(
        mini_lisp
        PROPERTIES CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON)
if (MSVC)
    target_compile_options(mini_lisp PRIVATE /utf-8 /Zc:preprocessor)
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

if (USE_GTEST)
    find_package(GTest REQUIRED)
    enable_testing()

    aux_source_directory(tests TEST_SOURCES)
    list(APPEND TEST_SOURCES ${SOURCES})
    # remove main.cpp
    list(REMOVE_ITEM TEST_SOURCES src/main.cpp)
    add_executable(mini_lisp_tests ${TEST_SOURCES})

    target_include_directories(mini_lisp_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(mini_lisp_tests GTest::GTest GTest::Main)
    if (USE_LLVM)
        if (DYNAMIC_LINK_LLVM)
            target_link_libraries(mini_lisp_tests LLVM)
        else ()
            target_link_libraries(mini_lisp_tests ${llvm_libs})
        endif ()
    endif ()
    set_target_properties(
            mini_lisp_tests
            PROPERTIES CXX_STANDARD 23
            CXX_STANDARD_REQUIRED ON)

    gtest_discover_tests(mini_lisp_tests)
endif ()

if (USE_LLVM)
    add_compile_definitions(-DUSE_LLVM)
    if (DYNAMIC_LINK_LLVM)
        target_link_libraries(mini_lisp LLVM)
    else ()
        target_link_libraries(mini_lisp ${llvm_libs})
    endif ()
endif ()