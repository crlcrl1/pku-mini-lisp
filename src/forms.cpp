#include "forms.h"

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
};
// clang-format on

ValuePtr defineForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.size() != 2) {
        throw ValueError(std::format("define: expected 2 arguments, but got {}", args.size()));
    }
    if (const auto name = args[0]->asSymbolName(); name.has_value()) {
        env.addVariable(*name, env.eval(args[1]));
    } else if (const auto pair = dynamic_cast<PairValue*>(args[0].get()); pair != nullptr) {
        const auto procName = pair->getCar()->asSymbolName();
        if (!procName.has_value()) {
            throw ValueError("define: expected a symbol as the first argument");
        }
        auto lambda = lambdaForm({pair->getCdr(), args[1]}, env);
        env.addVariable(*procName, lambda);
    } else {
        throw ValueError("define: expected a symbol or a pair as the first argument");
    }
    return std::make_shared<NilValue>();
}

ValuePtr quoteForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
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

ValuePtr lambdaForm(const std::vector<ValuePtr>& args, EvalEnv&) {
    if (args.size() != 2) {
        throw ValueError(std::format("lambda: expected 2 arguments, but got {}", args.size()));
    }
    const auto params = dynamic_cast<PairValue*>(args[0].get());
    const auto body = dynamic_cast<PairValue*>(args[1].get());
    if (params == nullptr || body == nullptr) {
        throw ValueError("lambda: expected a pair of parameters and a pair of body");
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

    auto bodyVec = body->toVector();
    removeTrailingNil(bodyVec);

    return std::make_shared<LambdaValue>(paramsList, bodyVec);
}