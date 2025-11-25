#include <any>
#include <cstdint>
#include <format>
#include <iostream>
#include <fstream>
#include <istream>
#include <optional>
#include <string>

#include "laxer.hpp"
#include "regex/regex.hpp"
#include "nfa.hpp"
#include "token.hpp"

int main(const int argc, const char** argv)
{
    (void) argc;
    (void) argv;

    laxer::laxer l;
    l.open_file("D:/proj/edl-laxer/edl_demo/numbers.edl");

    l.add_rule(
        "\\d+", 0,
        [](laxer::token& token) {
            token.set_token_value(
                static_cast<std::int32_t>(std::stoi(token.get_matched_text())));
            return true;
        },
        "numbers");
    l.add_rule("0x[a-fA-F0-9]+", 1, {}, "hex numbers");
    l.add_rule("0b[01]+", 2, {}, "bin numbers");
    l.add_rule("[ \r\n\t]", 3, [](laxer::token& token) { return false; }, "ignores");
    l.add_rule(".", 4, [](laxer::token& token) { return false; }, "error token");

    laxer::token token = l.next_token();

    while (token.get_token_id() != laxer::nfa::invalid_state) {
        std::cout << std::format("{}\n", token);

        if (const auto intPtr = std::get_if<std::int32_t>(&token.get_token_value())) {
            std::cout << std::format("token value: {}\n", *intPtr);
        }

        token = l.next_token();
    }

    return 0;
}
