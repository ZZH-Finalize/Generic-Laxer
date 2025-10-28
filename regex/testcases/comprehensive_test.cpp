#include <iostream>
#include <format>
#include <string_view>
#include "regex/regex.hpp"

void print_match(const regex::dfa& dfa, const std::string_view& str, const std::string& test_name)
{
    bool result = dfa.match(str);
    std::cout << std::format("{}: {} ({})\n", test_name, result, str);
}

int main(const int argc, const char** argv)
{
    std::cout << "=== 综合测试开始 ===" << std::endl;

    // 测试1: 简单字符串匹配
    std::cout << "\n--- 测试1: 简单字符串 ---" << std::endl;
    regex::dfa dfa1("abc");
    print_match(dfa1, "abc", "abc匹配abc");
    print_match(dfa1, "ab", "abc匹配ab");
    print_match(dfa1, "abcd", "abc匹配abcd");

    // 测试2: 单字符匹配
    std::cout << "\n--- 测试2: 单字符 ---" << std::endl;
    regex::dfa dfa2("x");
    print_match(dfa2, "x", "x匹配x");
    print_match(dfa2, "y", "x匹配y");

    // 测试3: 特殊字符（如果支持的话）
    std::cout << "\n--- 测试3: 点号通配符 ---" << std::endl;
    regex::dfa dfa3(".");
    print_match(dfa3, "a", ".匹配a");
    print_match(dfa3, "z", ".匹配z");
    print_match(dfa3, "1", ".匹配1");

    std::cout << "\n=== 综合测试结束 ===" << std::endl;

    return 0;
}
