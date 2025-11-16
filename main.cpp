#include <any>
#include <format>
#include <iostream>
#include <fstream>
#include <optional>
#include <string>

#include "regex/regex.hpp"
#include "nfa.hpp"
#include "token.hpp"

int main(const int argc, const char** argv)
{
    laxer::nfa nfa;

    auto return_print = [](laxer::token& token) {
        std::cout << std::format("action called\n{}", token) << std::endl;

        token.add_matched_text(" (Action edited)");

        return true;
    };

    nfa.add_nfa(regex::build_nfa("else"), return_print, "keyword else");

    auto dfa = nfa | regex::build | regex::minimize;

    auto res = dfa.match("else");

    if (res.has_value()) {
        std::cout << std::format("dfa.match return:\n{}", res.value()) << std::endl;
    }

    return 0;
}
