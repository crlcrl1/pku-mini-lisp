# Mini Lisp 解释器

[English](./README.md) | 简体中文

这是北京大学《软件设计实践》课程的大作业。

一个简单的 Lisp 解释器，使用 C++ 实现。

## 关键特性

- [x] 基本算术运算
- [x] 定义变量
- [x] 定义函数
- [x] 嵌套函数调用
- [x] 条件表达式
- [x] Lambda 表达式
- [x] 使用标记-清除算法进行垃圾回收
- [ ] 使用 LLVM 进行 JIT 编译
- [x] 使用 `require` 函数加载用 C++ 或 Lisp 编写的模块（Windows 不支持）
- [x] 带有语法高亮的 REPL

## 如何构建项目

这个项目使用 CMake 作为构建系统。你可以通过运行以下命令构建项目：

```bash
mkdir build
cd build
cmake .. && cmake --build .
```

这个项目使用 Google Test 作为测试框架，默认启用。如果你在配置 Google Test 或构建测试时遇到问题，你可以通过向 CMake 传递 `-DUSE_GTEST=OFF` 来禁用测试。如果你遇到其他问题，请先检查你的编译器版本。这个项目需要 C++23 支持。如果你的编译器确实支持 C++23 但仍然无法构建项目，请提交一个 issue。

## 许可证
这个项目根据 Apache 许可证第 2.0 版进行许可。有关详细信息，请参阅 [LICENSE](./LICENSE) 文件。

### 第三方许可证
这个项目使用 [replxx](https://github.com/AmokHuginnsson/replxx) 作为 readline 支持。许可证包含在 `third_party/replxx` 目录中。