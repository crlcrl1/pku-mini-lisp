#ifndef GC_H
#define GC_H

#include <concepts>

#include "value.h"

class ValuePool {
    std::vector<Value*> values;
    std::vector<EvalEnv*> envs;
    EvalEnv* rootEnv;

    static NilValue nil;

public:
    ValuePool();
    ~ValuePool();

    EvalEnv* makeEnv(const EvalEnv* parent);

    template <std::derived_from<Value> T, typename... Args>
    T* makeValue(Args&&... args);

    /**
     * Do garbage collection
     *
     * @return number of objects collected
     */
    size_t gc();

    EvalEnv* root() const;
};

extern ValuePool pool;

template <std::derived_from<Value> T, typename... Args>
T* ValuePool::makeValue(Args&&... args) {
    auto value = new T(std::forward<Args>(args)...);
    values.push_back(value);
    return value;
}

template <>
inline NilValue* ValuePool::makeValue<NilValue>() {
    return &nil;
}

#endif  // GC_H
