#include "pool.h"

#include <ranges>

#include "builtins.h"
#include "eval_env.h"

NilValue ValuePool::nil{};

ValuePool::ValuePool() {
    rootEnv = new EvalEnv();
    rootEnv->addBuiltins();
    envs.push_back(rootEnv);
}

ValuePool::~ValuePool() {
    for (const auto value : values) {
        delete value;
    }
    for (const auto env : envs) {
        delete env;
    }
}

template <>
NilValue* ValuePool::makeValue<NilValue>() {
    return &nil;
}

EvalEnv* ValuePool::makeEnv(const EvalEnv* parent) {
    const auto env = new EvalEnv(parent);
    envs.push_back(env);
    return env;
}

size_t ValuePool::gc() {
    size_t count = 0;

    std::set<Value*> reachableValues;
    std::set<EvalEnv*> reachableEnvs;
    std::vector<Value*> addedValues;
    std::vector<EvalEnv*> addedEnvs;

    // mark available values
    // first, we start from the root environment
    addedEnvs.push_back(rootEnv);
    bool found = false;

    do {
        found = false;
        // add new envs to reachableEnvs
        while (!addedEnvs.empty()) {
            auto env = addedEnvs.back();
            addedEnvs.pop_back();
            reachableEnvs.insert(env);
            for (const auto value : std::views::values(env->symbolTable)) {
                found = true;
                addedValues.push_back(value);
                const auto children = Value::children(value);
                addedValues.insert(addedValues.end(), children.begin(), children.end());
            }
        }
        // add new values to reachableValues, if they are lambdas, add their envs to addedEnvs
        while (!addedValues.empty()) {
            Value* value = addedValues.back();
            addedValues.pop_back();
            if (value == &nil) {
                continue;
            }
            reachableValues.insert(value);
            if (value->getType() == ValueType::LAMBDA) {
                const auto lambda = dynamic_cast<LambdaValue*>(value);
                if (!reachableEnvs.contains(lambda->env)) {
                    addedEnvs.push_back(lambda->env);
                }
                for (const auto v : lambda->body) {
                    addedValues.push_back(v);
                    const auto children = Value::children(v);
                    addedValues.insert(addedValues.end(), children.begin(), children.end());
                }
                found = true;
            }
        }
    } while (found);

    // remove unreachable values
    while (!values.empty()) {
        auto value = values.back();
        values.pop_back();
        if (reachableValues.contains(value)) {
            continue;
        }
        delete value;
        count++;
    }
    // remove unreachable envs
    while (!envs.empty()) {
        auto env = envs.back();
        envs.pop_back();
        if (reachableEnvs.contains(env)) {
            continue;
        }
        delete env;
        count++;
    }

    // add reachable values and envs back to the pool
    for (const auto value : reachableValues) {
        values.push_back(value);
    }
    for (const auto env : reachableEnvs) {
        envs.push_back(env);
    }

    return count;
}

EvalEnv* ValuePool::root() const {
    return rootEnv;
}

ValuePool pool;
