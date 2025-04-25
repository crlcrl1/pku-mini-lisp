#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "eval_env.h"
#include "parser.h"
#include "pool.h"
#include "tokenizer.h"
#include "value.h"

#ifdef USE_LLVM
#include "jit/jit.h"
#endif

int main(int argc, char* argv[]) {
    const auto env = pool.root();
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

#ifdef USE_LLVM
    jit::initializeLLVM(pool.root());
#endif

    int loop_cnt = 0;
    while (true) {
        loop_cnt++;
        try {
            auto tokens = Tokenizer::fromStream(input, isRepl);
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
        } catch (std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        if (isRepl) {
            pool.gc();
            continue;
        }
        if (loop_cnt % 16 == 0) {
            pool.gc();
        }
    }

    if (!isRepl) {
        delete input;
    }
#ifdef USE_LLVM
    jit::finalizeLLVM();
#endif
    return 0;
}
