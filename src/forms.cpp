#include "forms.h"

#include <filesystem>
#include <ranges>

#ifndef _WIN32
#include <dlfcn.h>
#endif

#include <fstream>

#include "error.h"
#include "eval_env.h"
#include "parser.h"
#include "pool.h"
#include "tokenizer.h"
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
    {"require", &requireForm}
};
// clang-format on

ValuePtr defineForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    if (params.size() < 2) {
        throw ValueError("define: expected at least two arguments", Location::fromRange(params));
    }
    if (const auto name = params[0]->asSymbolName(); name.has_value()) {
        env->addVariable(*name, env->eval(params[1]));
    } else if (const auto pair = dynamic_cast<PairValue*>(params[0]); pair != nullptr) {
        const auto procName = pair->getCar()->asSymbolName();
        if (!procName.has_value()) {
            throw ValueError("define: expected a symbol as the first argument",
                             pair->getCar()->getLocation());
        }
        std::vector procParams = {pair->getCdr()};
        procParams.insert(procParams.end(), params.begin() + 1, params.end());
        const auto lambda = lambdaForm(procParams, env);
        env->addVariable(*procName, lambda);
    } else {
        throw ValueError("define: expected a symbol or a pair as the first argument",
                         params[0]->getLocation());
    }
    return LISP_NIL;
}

ValuePtr quoteForm(const std::vector<ValuePtr>& params, EvalEnv*) {
    CHECK_PARAM_NUM(quote, 1);
    return params[0];
}

bool convertToBool(const ValuePtr& value) {
    if (value->getType() == ValueType::BOOLEAN) {
        return dynamic_cast<BooleanValue*>(value)->getValue();
    }
    return true;
}

ValuePtr ifForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    CHECK_PARAM_NUM(if, 3);
    if (const auto condition = env->eval(params[0]); convertToBool(condition)) {
        return env->eval(params[1]);
    }
    if (params.size() == 3) {
        return env->eval(params[2]);
    }
    return LISP_NIL;
}

ValuePtr andForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    for (const auto& param : params) {
        if (const auto value = env->eval(param); !convertToBool(value)) {
            return LISP_BOOL(false);
        }
    }
    const size_t len = params.size();
    if (len == 0) {
        return LISP_BOOL(true);
    }
    return env->eval(params[len - 1]);
}

ValuePtr orForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    for (const auto& arg : params) {
        if (const auto value = env->eval(arg); convertToBool(value)) {
            return value;
        }
    }
    return LISP_BOOL(false);
}

ValuePtr lambdaForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    CHECK_EMPTY_PARAMS(lambda);
    const auto lambdaParams = dynamic_cast<PairValue*>(params[0]);
    const auto body = std::vector(params.begin() + 1, params.end());
    if (lambdaParams == nullptr && params[0]->getType() != ValueType::NIL) {
        throw ValueError("lambda: expected a pair of parameters", params[0]->getLocation());
    }
    if (lambdaParams == nullptr) {  // no parameters
        return ValuePool::instance()->makeValue<LambdaValue>(std::vector<std::string>{}, body, env);
    }
    auto paramsList = std::vector<std::string>{};
    auto paramsVec = lambdaParams->toVector();
    removeTrailingNil(paramsVec);

    for (const auto& param : paramsVec) {
        if (const auto name = param->asSymbolName(); name.has_value()) {
            paramsList.push_back(*name);
        } else {
            throw ValueError("lambda: expected a list of symbols as parameters",
                             param->getLocation());
        }
    }

    return ValuePool::instance()->makeValue<LambdaValue>(paramsList, body, env);
}

ValuePtr evalForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    CHECK_PARAM_NUM(eval, 1);
    return env->eval(env->eval(params[0]));
}

ValuePtr condForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    CHECK_EMPTY_PARAMS(cond);
    const size_t len = params.size();
    for (auto [i, v] : std::views::enumerate(params)) {
        CHECK_TYPE(v, PAIR, cond, pair);
        const auto pair = dynamic_cast<PairValue*>(v);
        auto pairVec = pair->toVector();
        // check if the argument is a proper list
        CHECK_LIST(pairVec, cond);

        bool flag = false;
        ValuePtr result;
        if (i == len - 1 && pairVec[0]->asSymbolName() == "else") {
            flag = true;
            result = LISP_BOOL(true);
        } else {
            const auto cond = env->eval(pairVec[0]);
            if (cond->getType() != ValueType::BOOLEAN) {
                flag = true;  // any value other than #f is considered true
            } else {
                flag = dynamic_cast<BooleanValue*>(cond)->getValue();
            }
            result = cond;
        }
        if (flag) {
            if (pairVec.size() == 1) {  // no consequent, return the condition
                return result;
            }

            for (const auto& expr : std::vector(pairVec.begin() + 1, pairVec.end())) {
                result = env->eval(expr);
            }
            return result;
        }
        if (i == len - 1) {
            return result;
        }
    }
    throw InternalError("cond: unexpected error", std::nullopt);
}

ValuePtr beginForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    ValuePtr result = LISP_NIL;
    for (const auto& expr : params) {
        result = env->eval(expr);
    }
    return result;
}

ValuePtr letForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    CHECK_EMPTY_PARAMS(let);
    CHECK_TYPE(params[0], PAIR, let, pair);

    // add variables to the environment
    const auto varPair = dynamic_cast<PairValue*>(params[0]);
    auto varVec = varPair->toVector();
    if (varVec.back()->getType() != ValueType::NIL) {
        throw ValueError("let expected a list as the first argument", varPair->getLocation());
    }
    varVec.pop_back();
    std::unordered_map<std::string, ValuePtr> newVarMap;
    for (const auto& var : varVec) {
        CHECK_TYPE(var, PAIR, let, pair);
        const auto pair = dynamic_cast<PairValue*>(var);
        auto pairVec = pair->toVector();
        if (pairVec.size() != 3 || pairVec[2]->getType() != ValueType::NIL) {
            throw ValueError("let: expected a list as the argument", pair->getLocation());
        }
        auto name = pairVec[0]->asSymbolName();
        if (!name.has_value()) {
            throw ValueError("let: expected a symbol as the first element of the list",
                             pairVec[0]->getLocation());
        }
        auto value = env->eval(pairVec[1]);
        newVarMap[*name] = value;
    }
    // create a new environment
    auto newEnv = ValuePool::instance()->makeEnv(env);
    for (const auto& [name, value] : newVarMap) {
        newEnv->addVariable(name, value);
    }

    // evaluate the body
    ValuePtr result = LISP_NIL;
    for (const auto& expr : std::vector(params.begin() + 1, params.end())) {
        result = newEnv->eval(expr);
    }

    return result;
}

ValuePtr quasiquoteForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    CHECK_PARAM_NUM(quasiquote, 1);
    CHECK_TYPE(params[0], PAIR, quasiquote, pair);
    std::vector<ValuePtr> result;

    // convert the pair to a vector
    const auto argPair = dynamic_cast<PairValue*>(params[0]);
    auto argVec = argPair->toVector();
    if (argVec.empty()) {
        return params[0];
    }
    CHECK_LIST(argVec, quasiquote);
    for (const auto& arg : argVec) {
        if (arg->getType() == ValueType::PAIR) {
            const auto pair = dynamic_cast<PairValue*>(arg);
            if (pair->getCar()->asSymbolName() == "unquote") {
                CHECK_TYPE(pair->getCdr(), PAIR, quasiquote, pair);
                const auto unquoteArg = dynamic_cast<PairValue*>(pair->getCdr())->getCar();
                result.push_back(env->eval(unquoteArg));
                continue;
            }
        }
        result.push_back(arg);
    }
    return LISP_PAIR(PairValue::fromVector(result));
}

void loadExtension(const std::string& fileName) {
    using InitFuncType = void();

#ifdef _WIN32
    throw ValueError("Native extensions are not supported on Windows yet.", std::nullopt);
#else
    // convert to absolute path
    std::filesystem::path path(fileName);
    if (!path.is_absolute()) {
        path = std::filesystem::current_path() / path;
    }
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        throw ValueError(std::format("Failed to load extension: {}", dlerror()), std::nullopt);
    }
    auto initFunc = reinterpret_cast<InitFuncType*>(dlsym(handle, "init_ext"));
    if (const char* error = dlerror()) {
        dlclose(handle);
        throw ValueError(std::format("Failed to load extension: {}", error), std::nullopt);
    }
    initFunc();
#endif
}

std::vector<std::string> readLispPath() {
    std::vector<std::string> paths = {"."};
    const char* env = std::getenv("LISP_PATH");
    if (env == nullptr) {
        return paths;
    }

    std::string envPath(env);
    std::string delimiter = ":";
    size_t pos = 0;
    while ((pos = envPath.find(delimiter)) != std::string::npos) {
        paths.push_back(envPath.substr(0, pos));
        envPath.erase(0, pos + delimiter.length());
    }
    if (!envPath.empty()) {
        paths.push_back(envPath);
    }
    return paths;
}

ValuePtr requireForm(const std::vector<ValuePtr>& params, EvalEnv* env) {
    CHECK_PARAM_NUM(require, 1);
    CHECK_TYPE(params[0], STRING, require, string);
    const auto moduleName = dynamic_cast<StringValue*>(params[0])->getValue();
    if (std::ranges::find(EvalEnv::loadStack, moduleName) != EvalEnv::loadStack.end()) {
        throw ValueError(std::format("Circular dependency detected: {}", moduleName),
                         params[0]->getLocation());
    }
    EvalEnv::loadStack.push_back(moduleName);
    const auto filename = std::format("{}.scm", moduleName);
#ifdef _WIN32
    const auto extensionFilename = std::format("lib{}.dll", moduleName);
#else
    const auto extensionFilename = std::format("lib{}.so", moduleName);
#endif

    // find dynamic library or script file in LISP_PATH
    bool extensionFind = false;
    std::string extensionFile;
    bool moduleFind = false;
    std::string moduleFile;

    const auto paths = readLispPath();
    for (const auto& path : paths) {
        const auto modulePath = std::filesystem::path(path) / filename;
        const auto extensionPath = std::filesystem::path(path) / extensionFilename;

        if (std::filesystem::exists(modulePath)) {
            moduleFind = true;
            moduleFile = modulePath.string();
            break;
        }
        if (std::filesystem::exists(extensionPath)) {
            extensionFind = true;
            extensionFile = extensionPath.string();
            break;
        }
    }

    if (!extensionFind && !moduleFind) {
        throw ValueError(
            std::format(
                "Failed to load module {} because either {} or {} does not exist in LISP_PATH",
                filename, moduleName, extensionFilename),
            params[0]->getLocation());
    }

    if (extensionFind) {
        try {
            loadExtension(extensionFile);
        } catch (const ValueError& e) {
            throw ValueError(e.what(), params[0]->getLocation());
        }
        return LISP_NIL;
    }
    std::ifstream file(moduleFile);
    if (!file.is_open()) {
        throw ValueError(std::format("Failed to open file: {}", filename),
                         params[0]->getLocation());
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // parse the content
    auto tokens = Tokenizer::tokenize(content, moduleFile, 0);
    Parser parser(std::move(tokens));
    while (!parser.empty()) {
        env->eval(parser.parse());
    }
    return LISP_NIL;
}