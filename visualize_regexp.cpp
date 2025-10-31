#include <exception>
#include <format>
#include <iostream>
#include <fstream>
#include <string>

#include "regex/dfa.hpp"
#include "regex/nfa.hpp"
#include "regex/regex.hpp"

int main(const int argc, const char **argv)
{
    for (auto i = 1; i < argc; i++) {
        std::string exp(argv[i]);

        try {
            auto nfa           = regex::build_nfa(exp);
            auto dfa           = nfa | regex::to_dfa;
            auto minimized_dfa = dfa | regex::minimize;

            std::cout << std::format("regexp: `{}`\n\n", exp);
            std::cout << std::format("nfa:\n\n{:LR}\n\n", nfa);
            std::cout << std::format("dfa:\n\n{:LR}\n\n", dfa);
            std::cout << std::format("minimized_dfa:\n\n{:LR}\n\n", minimized_dfa);
        } catch (const std::exception &e) {
            std::cout << std::format(
                "error occured when convert regexp: {}\nexception:\t{}", exp, e.what());
        }
    }

    return 0;
}
