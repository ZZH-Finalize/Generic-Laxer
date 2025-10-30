#include <format>
#include <iostream>
#include <fstream>
#include <string>

#include "regex/nfa.hpp"
#include "regex/regex.hpp"

int main(const int argc, const char** argv)
{
    std::ifstream test_file(argv[1]);

    if (false == test_file.is_open()) {
        std::cout << "file open fail" << std::endl;
        return -1;
    }

    auto nfa1 = regex::build_nfa("[a-zA-Z_][a-zA-Z0-9_]*");
    auto nfa2 = regex::build_nfa("[0-9]+");

    auto combined_nfa = nfa1 | nfa2;

    std::cout << std::format("nfa1 final: {}\n", nfa1.get_final());
    std::cout << std::format("nfa2 final: {}\n", nfa2.get_final());
    std::cout << std::format("combined_nfa final: {}\n", combined_nfa.get_final());

    auto dfa = regex::build(combined_nfa) | regex::minimize;

    std::cout << std::format("state_num: {}\nfinal states:\n", dfa.get_state_count());

    for (auto state : dfa.get_final_states()) {
        std::cout << std::format("{}\n", state);
    }

    test_file.close();

    return 0;
}
