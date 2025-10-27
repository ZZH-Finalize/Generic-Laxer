#include <iostream>
#include <format>
#include "regex.hpp"

int main(const int argc, const char **argv)
{
    regex::nfa nfa1 = regex::nfa::from("abc");
    regex::nfa nfa2 = regex::nfa::from("a|b");

    std::cout << std::format("nfa1: {}\n", nfa1);
    std::cout << std::format("nfa1: {}\n", nfa2);

    return 0;
}
