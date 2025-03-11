#ifndef GC_H
#define GC_H

#include "value.h"

template <typename T, typename Base>
concept DerivedFrom = std::is_base_of_v<Base, T>;

class ValuePool {
    std::vector<Value*> values;
    std::vector<EvalEnv*> envs;
    EvalEnv* rootEnv;

public:
    ValuePool();
    ~ValuePool();

    EvalEnv* makeEnv(const EvalEnv* parent);

    template <DerivedFrom<Value> T, typename... Args>
    T* makeValue(Args&&... args) {
        auto value = new T(std::forward<Args>(args)...);
        values.push_back(value);
        return value;
    }

    /**
     * Do garbage collection
     *
     * @return number of objects collected
     */
    size_t gc();

    EvalEnv* root() const;
};

extern ValuePool pool;

#endif  // GC_H
