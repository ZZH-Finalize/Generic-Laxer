#include <iostream>
#include <format>
#include <string_view>
#include "regex.hpp"

void print_match_result(const regex::dfa& dfa, const std::string_view& str, const std::string& test_name, bool expected_full_match)
{
    bool full_match_result = dfa.match(str);
    std::string full_match_status = (full_match_result == expected_full_match) ? "PASS" : "FAIL";
    std::cout << std::format("[{}] {}: 完全匹配={} ({})\n", full_match_status, test_name, full_match_result, str);
    
    if (full_match_result != expected_full_match) {
        std::cout << "  -> 期望完全匹配结果: " << expected_full_match << ", 实际: " << full_match_result << std::endl;
        std::exit(1);
    }
}

void print_find_result(const regex::dfa& dfa, const std::string_view& str, const std::string& test_name, int expected_pos)
{
    int find_result = dfa.find_match(str);
    std::string find_status = (find_result == expected_pos) ? "PASS" : "FAIL";
    std::cout << std::format("[{}] {}: 查找匹配位置={} ({})\n", find_status, test_name, find_result, str);
    
    if (find_result != expected_pos) {
        std::cout << "  -> 期望查找匹配位置: " << expected_pos << ", 实际: " << find_result << std::endl;
        std::exit(1);
    }
}

int main(const int argc, const char** argv)
{
    std::cout << "=== 正则表达式完全匹配与查找匹配测试 ===" << std::endl;

    // 测试模式 "abc"
    std::cout << "\n--- 测试模式 'abc' ---" << std::endl;
    {
        regex::dfa dfa = regex::build("abc");
        
        // 完全匹配测试
        print_match_result(dfa, "abc", "abc完全匹配abc", true);
        print_match_result(dfa, "abcd", "abc完全匹配abcd", false);
        print_match_result(dfa, "xabc", "abc完全匹配xabc", false);
        print_match_result(dfa, "ab", "abc完全匹配ab", false);
        
        // 查找匹配测试
        print_find_result(dfa, "abc", "abc中查找abc", 0);      // 位置0匹配
        print_find_result(dfa, "abcd", "abcd中查找abc", 0);    // 位置0匹配
        print_find_result(dfa, "xabc", "xabc中查找abc", 1);    // 位置1匹配
        print_find_result(dfa, "xabcy", "xabcy中查找abc", 1);  // 位置1匹配
        print_find_result(dfa, "xyz", "xyz中查找abc", -1);     // 未找到匹配
        print_find_result(dfa, "ab", "ab中查找abc", -1);       // 未找到匹配
    }

    // 测试模式 "a"
    std::cout << "\n--- 测试模式 'a' ---" << std::endl;
    {
        regex::dfa dfa = regex::build("a");
        
        print_match_result(dfa, "a", "a完全匹配a", true);
        print_match_result(dfa, "aa", "a完全匹配aa", false);  // 整个aa不匹配模式a
        
        print_find_result(dfa, "a", "a中查找a", 0);      // 位置0匹配
        print_find_result(dfa, "aa", "aa中查找a", 0);    // 位置0匹配第一个a
        print_find_result(dfa, "ba", "ba中查找a", 1);    // 位置1匹配
        print_find_result(dfa, "bab", "bab中查找a", 1);  // 位置1匹配第一个a
        print_find_result(dfa, "xyz", "xyz中查找a", -1); // 未找到匹配
    }

    // 测试模式 "ab*c" (包含量词)
    std::cout << "\n--- 测试模式 'ab*c' ---" << std::endl;
    {
        regex::dfa dfa = regex::build("ab*c");
        
        print_match_result(dfa, "ac", "ab*c完全匹配ac", true);
        print_match_result(dfa, "abc", "ab*c完全匹配abc", true);
        print_match_result(dfa, "abbc", "ab*c完全匹配abbc", true);
        print_match_result(dfa, "abx", "ab*c完全匹配abx", false);
        print_match_result(dfa, "xabbc", "ab*c完全匹配xabbc", false);
        
        print_find_result(dfa, "ac", "ac中查找ab*c", 0);
        print_find_result(dfa, "abc", "abc中查找ab*c", 0);
        print_find_result(dfa, "abbc", "abbc中查找ab*c", 0);
        print_find_result(dfa, "xabbc", "xabbc中查找ab*c", 1);  // 从位置1开始匹配
        print_find_result(dfa, "xabc", "xabc中查找ab*c", 1);   // 从位置1开始匹配
        print_find_result(dfa, "abxyz", "abxyz中查找ab*c", -1); // 不匹配，因为没有以c结尾
        print_find_result(dfa, "abbcx", "abbcx中查找ab*c", 0);  // 从位置0开始匹配
    }

    std::cout << "\n=== 所有测试通过! ===" << std::endl;
    std::cout << "当前实现提供了两种匹配语义：" << std::endl;
    std::cout << "1. match() - 完全匹配：整个字符串必须匹配模式" << std::endl;
    std::cout << "2. find_match() - 查找匹配：在字符串中查找匹配模式的子串" << std::endl;

    return 0;
}
