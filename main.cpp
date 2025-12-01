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

    std::ifstream file("../../../../edl_demo/numbers.edl");
    laxer::laxer l(file.rdbuf());

    l.add_rule("\\d+", 0, laxer::converter::dec, "numbers");
    l.add_rule("0x[a-fA-F0-9]+", 1, laxer::converter::hex, "hex numbers");
    l.add_rule("0b[01]+", 2, laxer::converter::bin, "bin numbers");
    l.add_rule("[ \r\n\t]", 3, laxer::converter::ignore, "ignores");
    l.add_rule(".", 4, laxer::converter::ignore, "error token");

    laxer::token token = l.next_token();

    while (token.get_token_id() != laxer::nfa::invalid_state) {
        std::cout << std::format("{}\n", token);

        if (const auto intPtr = std::get_if<std::uint64_t>(&token.get_token_value())) {
            if (token.get_token_id() == 1) {
                std::cout << std::format("token value: 0x{:X}\n", *intPtr);
            } else {
                std::cout << std::format("token value: {}\n", *intPtr);
            }
        }

        token = l.next_token();
    }

    file.close();

    return 0;
}
