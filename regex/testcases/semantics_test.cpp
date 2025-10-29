#include <iostream>
#include <format>
#include <string_view>
#include "regex/regex.hpp"

void print_match(const regex::dfa& dfa, const std::string_view& str, const std::string& test_name, bool expected)
{
    bool result = dfa.match(str);
    std::string status = (result == expected) ? "PASS" : "FAIL";
    std::cout << std::format("[{}] {}: {} ({})\n", status, test_name, result, str);
    
    if (result != expected) {
        std::cout << "  -> 期望: " << expected << ", 实际: " << result << std::endl;
        std::exit(1); // 测试失败，退出程序
    }
}

int main(const int argc, const char** argv)
{
    std::cout << "=== 正则表达式语义测试 ===" << std::endl;
    std::cout << "当前实现似乎是完全匹配语义（整个字符串必须匹配模式）\n" << std::endl;

    // 测试完全匹配语义
    std::cout << "--- 完全匹配语义测试 ---" << std::endl;
    {
        regex::dfa dfa = regex::build("abc");
        print_match(dfa, "abc", "abc完全匹配abc", true);
        print_match(dfa, "abcd", "abc完全匹配abcd", false);  // 应该是false，因为整个字符串不匹配
        print_match(dfa, "xabc", "abc完全匹配xabc", false);  // 应该是false
        print_match(dfa, "ab", "abc完全匹配ab", false);     // 应该是false
    }

    std::cout << "\n--- 其他模式测试 ---" << std::endl;
    {
        regex::dfa dfa = regex::build("a");
        print_match(dfa, "a", "a完全匹配a", true);
        print_match(dfa, "aa", "a完全匹配aa", false);  // 应该是false，因为整个字符串是"aa"不匹配模式"a"
        print_match(dfa, "b", "a完全匹配b", false);
    }

    std::cout << "\n--- 量词语义测试 ---" << std::endl;
    {
        regex::dfa dfa = regex::build("ab*c");  // a后跟零个或多个b，然后是c
        print_match(dfa, "ac", "ab*c完全匹配ac", true);
        print_match(dfa, "abc", "ab*c完全匹配abc", true);
        print_match(dfa, "abbc", "ab*c完全匹配abbc", true);
        print_match(dfa, "abx", "ab*c完全匹配abx", false);  // 不以c结尾
        print_match(dfa, "xabbc", "ab*c完全匹配xabbc", false);  // 不以a开头
        print_match(dfa, "abbcx", "ab*c完全匹配abbcx", false);  // 不完全匹配，多了x
    }

    std::cout << "\n=== 完全匹配语义测试通过! ===" << std::endl;
    std::cout << "注意：当前实现是完全匹配语义，即整个输入字符串必须完全匹配模式" << std::endl;
    std::cout << "如果需要子串匹配功能（查找匹配），则需要额外实现。" << std::endl;

    return 0;
}
