#include "types.h"

jit::RefCountList::RefCountList(size_t size)
    : data(new double[size]), refCount(new size_t), ptr(data) {
    *refCount = 1;
}

jit::RefCountList::RefCountList(const RefCountList& other) {
    if (this == &other) {
        return;
    }
    refCount = other.refCount;
    data = other.data;
    ptr = other.ptr;
    (*refCount)++;
}

jit::RefCountList::~RefCountList() {
    if (--*refCount == 0) {
        delete[] data;
        delete refCount;
    }
}

double jit::RefCountList::car() const {
    return ptr[0];
}

jit::RefCountList jit::RefCountList::cdr() const {
    RefCountList rcl;
    rcl.data = data;
    rcl.refCount = refCount;
    rcl.ptr = ptr + 1;
    (*refCount)++;
    return rcl;
}

double& jit::RefCountList::operator[](size_t idx) {
    return ptr[idx];
}

const double& jit::RefCountList::operator[](size_t idx) const {
    return ptr[idx];
}

jit::RefCountList* create_list(size_t size) {
    return new jit::RefCountList(size);
}

double car(const jit::RefCountList* list) {
    return list->car();
}

jit::RefCountList* cdr(const jit::RefCountList* list) {
    return new jit::RefCountList(list->cdr());
}

void delete_list(const jit::RefCountList* list) {
    delete list;
}

jit::RefCountList* copy_list(const jit::RefCountList* list) {
    return new jit::RefCountList(*list);
}

double get_value(const jit::RefCountList* list, size_t idx) {
    return (*list)[idx];
}