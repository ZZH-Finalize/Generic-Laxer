#include <cstdint>
#include <format>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "laxer.hpp"
#include "token.hpp"

enum tokens
{
    operators,

    number,

    space,
    error,
};

bool debug(laxer::token &token) noexcept
{
    std::cout << std::format("error token: {}", token.get_matched_text());

    return true;
}

std::uint64_t parse_number(laxer::laxer &l)
{
    auto token = l.next_token();

    if (token.get_token_id() != tokens::number) {
        throw std::runtime_error("expect a number");
    }

    return std::get<std::uint64_t>(token.get_token_value());
}

char parse_op(laxer::laxer &l)
{
    auto token = l.next_token();

    if (tokens::operators != token.get_token_id()) {
        // throw std::runtime_error("expect a operator");
        return 'E';
    }

    return std::get<std::string>(token.get_token_value()).front();
}

std::uint64_t parse_exp(laxer::laxer &l)
{
    auto left = parse_number(l);
    auto op   = parse_op(l);

    if ('E' == op) {
        return left;
    }

    auto right = parse_exp(l);

    std::uint64_t res = 0;

    switch (op) {
        case '+': res = left + right; break;
        case '-': res = left - right; break;
        case '*': res = left * right; break;
        case '/': res = left / right; break;

        default: throw std::runtime_error(std::format("unknow operator: {}", op));
    }

    return res;
}

int main(const int argc, const char **argv)
{
    std::stringbuf sb(argv[1]);
    std::cout << std::format("calculate {}", argv[1]);
    laxer::laxer l(&sb);

    l.add_rule("+", tokens::operators, laxer::converter::string, "op dec");
    l.add_rule("-", tokens::operators, laxer::converter::string, "op add");
    l.add_rule("*", tokens::operators, laxer::converter::string, "op mul");
    l.add_rule("/", tokens::operators, laxer::converter::string, "op div");

    l.add_rule("\\d+", tokens::number, laxer::converter::dec, "numbers");
    l.add_rule("0x[a-fA-F0-9]+", tokens::number, laxer::converter::hex, "hex numbers");
    l.add_rule("0b[01]+", tokens::number, laxer::converter::bin, "bin numbers");
    l.add_rule("[ \r\n\t]", tokens::space);
    l.add_rule(".", tokens::error, debug, "error token");

    std::cout << std::format("={}", parse_exp(l));

    return 0;
}
