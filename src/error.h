#ifndef ERROR_H
#define ERROR_H

#include <stdexcept>

#include "location.h"

class LispError : public std::runtime_error {
    std::optional<Location> loc;
    std::string name_;

public:
    LispError(const std::string& msg, const std::optional<Location>& loc, const std::string& name)
        : std::runtime_error(msg), loc(loc), name_(name) {}

    const std::optional<Location>& location() const {
        return loc;
    }

    const std::string& name() const {
        return name_;
    }
};

#define LISP_ERROR(error_name)                                                 \
    class error_name : public LispError {                                      \
    public:                                                                    \
        error_name(const std::string& msg, const std::optional<Location>& loc) \
            : LispError(msg, loc, #error_name) {}                              \
    };

LISP_ERROR(SyntaxError)
LISP_ERROR(ValueError)
LISP_ERROR(UnimplementedError)
LISP_ERROR(InternalError)
LISP_ERROR(TypeError)

#undef LISP_ERROR

#endif