#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>

#include "error.h"
#include "eval_env.h"
#include "parser.h"
#include "pool.h"
#include "tokenizer.h"
#include "value.h"

int main(int argc, char* argv[]) {
    const auto env = ValuePool::instance()->root();
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [script]" << std::endl;
    }

    // initialize input stream
    std::istream* input;
    std::optional<std::string> file;
    bool isRepl;
    if (argc == 2) {
        try {
            input = new std::ifstream(argv[1]);
            file = argv[1];
            isRepl = false;
        } catch (std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            ValuePool::dispose();
            return 1;
        }
        if (!input->good()) {
            std::cerr << std::format("Error: failed to open file {}", argv[1]) << std::endl;
            ValuePool::dispose();
            return 1;
        }
    } else {
        input = &std::cin;
        file = std::nullopt;
        isRepl = true;
    }

    int row = 0;

    int loopCnt = 0;
    while (true) {
        loopCnt++;
        EvalEnv::loadStack = {};
        try {
            auto tokens = Tokenizer::fromStream(input, isRepl, file, row);
            if (tokens.empty()) {
                if (input->eof()) {  // Ctrl-D or end of file, exit
                    break;
                }
                continue;
            }

            Parser parser(std::move(tokens));
            while (!parser.empty()) {
                const auto value = parser.parse();
                const auto result = env->eval(value);
                if (isRepl) {
                    std::cout << result->toString() << std::endl;
                }
            }
        } catch (LispError& e) {
            std::cerr << std::format("{}:\n {}", e.name(), e.what()) << std::endl;
            if (auto& location = e.location()) {
                std::cerr << std::format("  {}:{}:{}", location->file, location->row, location->col)
                          << std::endl;
            }
            if (!isRepl) {
                ValuePool::dispose();
                return 1;
            }
        } catch (std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            if (!isRepl) {
                ValuePool::dispose();
                return 1;
            }
        }

        if (isRepl || loopCnt % 32 == 0) {
            ValuePool::instance()->gc();
        }
    }

    if (!isRepl) {
        delete input;
    }
    ValuePool::dispose();
    return 0;
}
