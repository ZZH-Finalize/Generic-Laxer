#include <iostream>
#include <format>
#include <string_view>
#include <cassert>
#include "regex.hpp"

void print_match(const regex::dfa& dfa, const std::string_view& str)
{
    auto result = dfa.match(str);
    std::cout << std::format("{}: {}\n", str, result.has_value());
}

int main(const int argc, const char** argv)
{
    std::cout << "=== 调试测试开始 ===" << std::endl;
    
    // 基本测试
    std::cout << "=== 基本测试 ===" << std::endl;
    {
        regex::dfa dfa = regex::build_dfa("hello world");
        auto result1 = dfa.match("hello world");
        print_match(dfa, "hello world");
        assert(result1.has_value());
        
        auto result2 = dfa.match("hello");
        print_match(dfa, "hello");
        assert(!result2.has_value());
        
        auto result3 = dfa.match("world");
        print_match(dfa, "world");
        assert(!result3.has_value());
        
        auto result4 = dfa.match("hello worlds");
        print_match(dfa, "hello worlds");
        assert(!result4.has_value());
        
        auto result5 = dfa.match("say hello world now");
        print_match(dfa, "say hello world now");
        assert(!result5.has_value());
    }

    // 单字符测试
    std::cout << "\n=== 单字符测试 ===" << std::endl;
    {
        regex::dfa dfa_single = regex::build_dfa("a");
        auto result6 = dfa_single.match("a");
        print_match(dfa_single, "a");
        assert(result6.has_value());
        
        auto result7 = dfa_single.match("b");
        print_match(dfa_single, "b");
        assert(!result7.has_value());
        
        auto result8 = dfa_single.match("aa");
        print_match(dfa_single, "aa");
        assert(!result8.has_value());
    }

    // 简单模式测试
    std::cout << "\n=== 简单模式测试 ===" << std::endl;
    {
        regex::dfa dfa_simple = regex::build_dfa("ab");
        auto result9 = dfa_simple.match("ab");
        print_match(dfa_simple, "ab");
        assert(result9.has_value());
        
        auto result10 = dfa_simple.match("a");
        print_match(dfa_simple, "a");
        assert(!result10.has_value());
        
        auto result11 = dfa_simple.match("b");
        print_match(dfa_simple, "b");
        assert(!result11.has_value());
        
        auto result12 = dfa_simple.match("abc");
        print_match(dfa_simple, "abc");
        assert(!result12.has_value());
    }

    std::cout << "\n=== 所有调试测试通过! ===" << std::endl;

    return 0;
}
