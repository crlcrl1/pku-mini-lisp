#ifndef TYPES_H
#define TYPES_H

#include <cstddef>

namespace jit {

class RefCountList {
    double* data = nullptr;
    size_t* refCount = nullptr;
    double* ptr = nullptr;

    RefCountList() = default;

public:
    explicit RefCountList(size_t size);
    RefCountList(const RefCountList& other);
    ~RefCountList();

    double car() const;
    RefCountList cdr() const;

    double& operator[](size_t idx);
    const double& operator[](size_t idx) const;
};

}  // namespace jit

extern "C" {

jit::RefCountList* create_list(size_t size);

double car(const jit::RefCountList* list);

jit::RefCountList* cdr(const jit::RefCountList* list);

void delete_list(const jit::RefCountList* list);

jit::RefCountList* copy_list(const jit::RefCountList* list);

double get_value(const jit::RefCountList* list, size_t idx);
}

#endif  // TYPES_H
