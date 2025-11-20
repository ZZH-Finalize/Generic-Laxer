#include <any>
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

    l.add_rule("\\d+", 0, {}, "numbers");
    l.add_rule("0x[a-fA-F0-9]+", 1, {}, "hex numbers");
    l.add_rule("0b[01]+", 1, {}, "bin numbers");
    l.add_rule("[ \r\n\t]", 2, [](laxer::token& token) { return false; }, "ignores");
    l.add_rule(".", 3, [](laxer::token& token) { return false; }, "error token");

    laxer::token token = l.next_token();

    while (token.get_token_id() != laxer::nfa::invalid_state) {
        std::cout << std::format("{}\n", token);
        token = l.next_token();
    }

    return 0;
}
