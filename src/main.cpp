#include <iostream>
#include <memory>
#include <string>

#include "eval_env.h"
#include "parser.h"
#include "tokenizer.h"
#include "value.h"

int main() {
    const auto env = EvalEnv::createEnv();
    while (true) {
        try {
            std::cout << ">>> ";
            std::string line;
            std::getline(std::cin, line);
            if (std::cin.eof()) {
                std::exit(0);
            }
            auto tokens = Tokenizer::tokenize(line);
            if (tokens.empty()) {
                continue;
            }
            Parser parser(std::move(tokens));
            const auto value = parser.parse();
            const auto result = env->eval(value);
            std::cout << result->toString() << std::endl;
        } catch (std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}
