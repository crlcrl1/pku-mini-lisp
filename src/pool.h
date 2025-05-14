#ifndef GC_H
#define GC_H

#include <concepts>

#include "value.h"

class ValuePool {
    std::vector<Value*> values;
    std::vector<EvalEnv*> envs;
    EvalEnv* rootEnv;

    static NilValue nil;
    ValuePool();

    static ValuePool* instance_;

public:
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

    static ValuePool* instance();
    static void dispose();
};

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
