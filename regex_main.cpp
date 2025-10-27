#include <iostream>
#include <format>
#include <string_view>
#include "regex.hpp"

void print_match(const regex::dfa& dfa, const std::string_view& str)
{
    std::cout<<std::format("{}: {}\n", str, dfa.match(str));
}

int main(const int argc, const char **argv)
{
    regex::nfa nfa1 = regex::nfa::from("abc");
    regex::nfa nfa2 = regex::nfa::from("aabb");

    regex::dfa dfa2(nfa2);

    print_match(dfa2, "aabb");

    return 0;
}
