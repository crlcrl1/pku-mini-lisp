# Mini Lisp Interpreter

English | [简体中文](./README.zh-CN.md)

This is the final project for course **Software Design** in PKU.

A simple Lisp interpreter implemented in C++.

## Features

- [x] Basic arithmetic operations
- [x] Define variables
- [x] Define functions
- [x] Nested function calls
- [x] Conditional expressions
- [x] Lambda expressions
- [x] Garbage collection with mark-and-sweep algorithm
- [ ] JIT compilation with LLVM
- [x] Load modules written in C++ or Lisp with `require` function (C++ extension is not supported on Windows)
- [x] REPL with syntax highlighting
- [x] Error handling with source code location information

## How to build the project

This project uses CMake as the build system. You can build the project by running the following commands:

```bash
mkdir build
cd build
cmake .. && cmake --build .
```

This project uses Google Test as the testing framework and it is enabled by default. You have trouble configuring Google Test or building tests, you can disable the tests by passing `-DUSE_GTEST=OFF` to CMake. If you encounter other problems, check your compiler version first. This project requires C++23 support. If your compiler do support C++23 and you still have trouble building the project, feel free to open an issue.

## License
This project is licensed under the Apache License, Version 2.0. See the [LICENSE](./LICENSE) file for details.

### Third-party Licenses
This project uses [replxx](https://github.com/AmokHuginnsson/replxx) for readline support. The license is included in the `third_party/replxx` directory.
