#include <iostream>
#include <format>
#include <string_view>
#include "regex/regex.hpp"

void print_match(const regex::dfa& dfa, const std::string_view& str)
{
    std::cout << std::format("{}: {}\n", str, dfa.match(str));
}

int main(const int argc, const char** argv)
{
    regex::dfa dfa = regex::build_dfa("hello world");

    print_match(dfa, "hello world");

    return 0;
}
