#include <any>
#include <cassert>
#include <format>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "dfa.hpp"
#include "nfa.hpp"
#include "regex/regex.hpp"

using rule_id_t = laxer::nfa::id_t;

const rule_id_t invalid_id = std::numeric_limits<rule_id_t>::max();

int main(const int argc, const char** argv)
{
    std::vector<std::string> rules = {
        "[0-9]+", "if", "else", "int", "[a-z]+", "[_a-zA-Z]+[a-zA-Z0-9]*",
    };

    std::vector<std::pair<std::string, rule_id_t>> tests = {
        {"6456546",     0         },
        {"if",          1         },
        {"else",        2         },
        {"int",         3         },
        {"ifelse",      4         },
        {"djfhkjh",     4         },
        {"djfhkjh7",    5         },
        {"_djfhkjh",    5         },
        {"asd465",      5         },
        {"_465",        5         },

        {"465awreawsd", invalid_id},
        {",./;'",       invalid_id},
    };

    // setup rules
    laxer::nfa laxer_nfa;
    for (const auto& rule : rules) {
        auto nfa = regex::build_nfa(rule);

        laxer_nfa.add_nfa(nfa);
    }

    auto dfa = regex::build(laxer_nfa);

    // check matchs
    for (const auto& [text, expected_id] : tests) {
        const auto& res = dfa.match(text);

        // matched
        if (res.has_value()) {
            auto matched_id = res->get_rule_id();

            std::cout << std::format("{} matched with rule {}, expected: {}\n", text,
                                     matched_id, expected_id);

            assert(matched_id == expected_id);
        } else { // unmatched
            std::cout << std::format("no rules matched for {}\n", text);
            assert(expected_id == invalid_id);
        }
    }

    return 0;
}
