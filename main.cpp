#include <any>
#include <format>
#include <iostream>
#include <fstream>
#include <optional>
#include <string>

#include "nfa.hpp"
#include "regex/regex.hpp"

int main(const int argc, const char** argv)
{
    regex::nfa rule1 = regex::build_nfa("[a-z]+");
    regex::nfa rule2 = regex::build_nfa("[0-9]+");

    laxer::nfa nfa;

    nfa.add_nfa({rule1, rule2});

    auto dfa       = nfa | regex::to_dfa;
    auto rule1_dfa = rule1 | regex::to_dfa;

    auto res = dfa.match("125");

    // 匹配
    if (res != std::nullopt) {
        if (res.has_value()) {
            std::cout << "metadata: "
                      << std::any_cast<laxer::nfa::rule_id_t>(res.value().get_metadata())
                      << std::endl;
        } else {
            std::cout << "no metadata" << std::endl;
        }
    }

    return 0;
}
