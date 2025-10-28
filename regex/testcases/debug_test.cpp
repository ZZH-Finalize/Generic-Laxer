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
    // 基本测试
    std::cout << "=== 基本测试 ===" << std::endl;
    regex::dfa dfa("hello world");
    print_match(dfa, "hello world");
    print_match(dfa, "hello");
    print_match(dfa, "world");
    print_match(dfa, "hello worlds");
    print_match(dfa, "say hello world now");

    // 单字符测试
    std::cout << "\n=== 单字符测试 ===" << std::endl;
    regex::dfa dfa_single("a");
    print_match(dfa_single, "a");
    print_match(dfa_single, "b");
    print_match(dfa_single, "aa");

    // 简单模式测试
    std::cout << "\n=== 简单模式测试 ===" << std::endl;
    regex::dfa dfa_simple("ab");
    print_match(dfa_simple, "ab");
    print_match(dfa_simple, "a");
    print_match(dfa_simple, "b");
    print_match(dfa_simple, "abc");

    return 0;
}
