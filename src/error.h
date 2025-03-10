#ifndef ERROR_H
#define ERROR_H

#include <stdexcept>

class SyntaxError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class ValueError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class UnimplementedError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class InternalError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class TypeError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

#endif