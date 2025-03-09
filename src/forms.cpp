#include "forms.h"

#include <ranges>

#include "error.h"
#include "utils.h"
#include "value.h"

// clang-format off
const std::unordered_map<std::string, SpecialFormType*> SPECIAL_FORMS = {
    {"define", &defineForm},
    {"quote", &quoteForm},
    {"if", &ifForm},
    {"and", &andForm},
    {"or", &orForm},
    {"lambda", &lambdaForm},
    {"eval", &evalForm},
    {"cond", &condForm},
    {"begin", &beginForm},
    {"let", &letForm},
    {"quasiquote", &quasiquoteForm},
};
// clang-format on

ValuePtr defineForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (const auto name = args[0]->asSymbolName(); name.has_value()) {
        env.addVariable(*name, env.eval(args[1]));
    } else if (const auto pair = dynamic_cast<PairValue*>(args[0].get()); pair != nullptr) {
        const auto procName = pair->getCar()->asSymbolName();
        if (!procName.has_value()) {
            throw ValueError("define: expected a symbol as the first argument");
        }
        std::vector params = {pair->getCdr()};
        params.insert(params.end(), args.begin() + 1, args.end());
        const auto lambda = lambdaForm(params, env);
        env.addVariable(*procName, lambda);
    } else {
        throw ValueError("define: expected a symbol or a pair as the first argument");
    }
    return std::make_shared<NilValue>();
}

ValuePtr quoteForm(const std::vector<ValuePtr>& args, EvalEnv&) {
    if (args.size() != 1) {
        throw ValueError(std::format("quote: expected 1 argument, but got {}", args.size()));
    }
    return args[0];
}

bool convertToBool(const ValuePtr& value) {
    if (value->getType() == ValueType::BOOLEAN) {
        return dynamic_cast<BooleanValue*>(value.get())->getValue();
    }
    return true;
}

ValuePtr ifForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.size() != 3) {
        throw ValueError(std::format("if: expected 3 arguments, but got {}", args.size()));
    }
    if (const auto condition = env.eval(args[0]); convertToBool(condition)) {
        return env.eval(args[1]);
    }
    if (args.size() == 3) {
        return env.eval(args[2]);
    }
    return std::make_shared<NilValue>();
}

ValuePtr andForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    for (const auto& arg : args) {
        if (const auto value = env.eval(arg); !convertToBool(value)) {
            return std::make_shared<BooleanValue>(false);
        }
    }
    const size_t len = args.size();
    if (len == 0) {
        return std::make_shared<BooleanValue>(true);
    }
    return env.eval(args[len - 1]);
}

ValuePtr orForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    for (const auto& arg : args) {
        if (const auto value = env.eval(arg); convertToBool(value)) {
            return value;
        }
    }
    return std::make_shared<BooleanValue>(false);
}

ValuePtr lambdaForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    const auto params = dynamic_cast<PairValue*>(args[0].get());
    const auto body = std::vector(args.begin() + 1, args.end());
    if (params == nullptr) {
        throw ValueError("lambda: expected a pair of parameters");
    }
    auto paramsList = std::vector<std::string>{};
    auto paramsVec = params->toVector();
    removeTrailingNil(paramsVec);

    for (const auto& param : paramsVec) {
        if (const auto name = param->asSymbolName(); name.has_value()) {
            paramsList.push_back(*name);
        } else {
            throw ValueError("lambda: expected a list of symbols as parameters");
        }
    }

    return std::make_shared<LambdaValue>(paramsList, body, env.createChild(paramsList, paramsVec));
}

ValuePtr evalForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.size() != 1) {
        throw ValueError(std::format("eval: expected 1 argument, but got {}", args.size()));
    }
    return env.eval(env.eval(args[0]));
}

ValuePtr condForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    const size_t len = args.size();
    if (len == 0) {
        throw ValueError("cond: expected at least 1 argument");
    }
    for (auto [i, v] : std::views::enumerate(args)) {
        if (v->getType() != ValueType::PAIR) {
            throw ValueError(std::format("cond: expected a pair as the {}-th argument", i + 1));
        }
        const auto pair = dynamic_cast<PairValue*>(v.get());
        auto pairVec = pair->toVector();
        // check if the argument is a proper list
        if (pairVec.empty()) {
            throw ValueError("cond: expected a non-empty list as the argument");
        }
        if (pairVec.back()->getType() != ValueType::NIL) {
            throw ValueError("cond: expected a list as the argument");
        }
        pairVec.pop_back();

        bool flag = false;
        if (i == len - 1 && pairVec[0]->asSymbolName() == "else") {
            flag = true;
        } else {
            const auto cond = env.eval(pairVec[0]);
            if (cond->getType() != ValueType::BOOLEAN) {
                throw ValueError("cond: expected a boolean value as the condition");
            }
            flag = dynamic_cast<BooleanValue*>(cond.get())->getValue();
        }
        if (flag) {
            if (pairVec.size() == 1) {
                return std::make_shared<BooleanValue>(true);
            }
            ValuePtr result = std::make_shared<NilValue>();
            for (const auto& expr : std::vector(pairVec.begin() + 1, pairVec.end())) {
                result = env.eval(expr);
            }
            return result;
        }
        if (i == len - 1) {
            return std::make_shared<BooleanValue>(flag);
        }
    }
    throw InternalError("cond: unexpected error");
}

ValuePtr beginForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    ValuePtr result = std::make_shared<NilValue>();
    for (const auto& expr : args) {
        result = env.eval(expr);
    }
    return result;
}

ValuePtr letForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.empty()) {
        throw ValueError("let: expected at least 1 argument");
    }
    if (args[0]->getType() != ValueType::PAIR) {
        throw ValueError("let: expected a pair as the first argument");
    }

    // add variables to the environment
    const auto varPair = dynamic_cast<PairValue*>(args[0].get());
    auto varVec = varPair->toVector();
    if (varVec.back()->getType() != ValueType::NIL) {
        throw ValueError("let: expected a list as the first argument");
    }
    varVec.pop_back();
    std::unordered_map<std::string, std::optional<ValuePtr>> oldVarMap;
    std::unordered_map<std::string, ValuePtr> newVarMap;
    for (const auto& var : varVec) {
        if (var->getType() != ValueType::PAIR) {
            throw ValueError("let: expected a list of pairs as the first argument");
        }
        const auto pair = dynamic_cast<PairValue*>(var.get());
        auto pairVec = pair->toVector();
        if (pairVec.size() != 3 || pairVec[2]->getType() != ValueType::NIL) {
            throw ValueError("let: expected a pair as the argument");
        }
        auto name = pairVec[0]->asSymbolName();
        if (!name.has_value()) {
            throw ValueError("let: expected a symbol as the first element of the pair");
        }
        auto value = env.eval(pairVec[1]);
        newVarMap[*name] = value;
    }

    // replace the environment variables
    for (const auto& [name, value] : newVarMap) {
        const auto oldValue = env.addVariable(name, value);
        oldVarMap[name] = oldValue;
    }

    // evaluate the body
    ValuePtr result = std::make_shared<NilValue>();
    for (const auto& expr : std::vector(args.begin() + 1, args.end())) {
        result = env.eval(expr);
    }

    // restore the environment
    for (const auto& [name, value] : oldVarMap) {
        if (value.has_value()) {
            env.addVariable(name, value.value());
        } else {
            env.removeVariable(name);
        }
    }

    return result;
}

ValuePtr quasiquoteForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    std::vector<ValuePtr> result;
    if (args.size() != 1) {
        throw ValueError("quasiquote: expected 1 argument");
    }
    if (args[0]->getType() != ValueType::PAIR) {
        return args[0];
    }
    const auto argPair = dynamic_cast<PairValue*>(args[0].get());
    auto argVec = argPair->toVector();
    if (argVec.empty()) {
        return args[0];
    }
    if (argVec.back()->getType() != ValueType::NIL) {
        throw ValueError("quasiquote: expected a list as the argument");
    }
    argVec.pop_back();
    for (const auto& arg : argVec) {
        if (arg->getType() == ValueType::PAIR) {
            const auto pair = dynamic_cast<PairValue*>(arg.get());
            if (pair->getCar()->asSymbolName() == "unquote") {
                if (pair->getCdr()->getType() != ValueType::PAIR) {
                    throw ValueError("quasiquote: expected a pair as the argument of unquote");
                }
                const auto unquoteArg = dynamic_cast<PairValue*>(pair->getCdr().get())->getCar();
                result.push_back(env.eval(unquoteArg));
                continue;
            }
        }
        result.push_back(arg);
    }
    return std::make_shared<PairValue>(PairValue::fromVector(result));
}
