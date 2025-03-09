#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "eval_env.h"
#include "parser.h"
#include "tokenizer.h"
#include "value.h"

int main(int argc, char* argv[]) {
    const auto env = EvalEnv::createEnv();
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [script]" << std::endl;
    }
    std::istream* input;
    bool isRepl;
    if (argc == 2) {
        try {
            input = new std::ifstream(argv[1]);
            isRepl = false;
        } catch (std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
        if (!input->good()) {
            std::cerr << std::format("Error: failed to open file {}", argv[1]) << std::endl;
            return 1;
        }
    } else {
        input = &std::cin;
        isRepl = true;
    }

    while (true) {
        try {
            auto tokens = Tokenizer::fromStream(input, isRepl);
            if (tokens.empty()) {
                if (input->eof()) {  // Ctrl-D or end of file, exit
                    break;
                }
                continue;
            }

            Parser parser(std::move(tokens));
            const auto value = parser.parse();
            const auto result = env->eval(value);
            if (isRepl) {
                std::cout << result->toString() << std::endl;
            }
            if (input->eof()) {
                break;
            }
        } catch (std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    if (!isRepl) {
        delete input;
    }
    return 0;
}
