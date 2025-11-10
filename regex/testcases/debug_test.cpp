#include <iostream>
#include <format>
#include <string_view>
#include <cassert>
#include "regex.hpp"

void print_match(const regex::dfa& dfa, const std::string_view& str)
{
    std::cout << std::format("{}: {}\n", str, dfa.match(str));
}

int main(const int argc, const char** argv)
{
    std::cout << "=== 调试测试开始 ===" << std::endl;
    
    // 基本测试
    std::cout << "=== 基本测试 ===" << std::endl;
    {
        regex::dfa dfa = regex::build_dfa("hello world");
        bool result1 = dfa.match("hello world");
        print_match(dfa, "hello world");
        assert(result1 == true);
        
        bool result2 = dfa.match("hello");
        print_match(dfa, "hello");
        assert(result2 == false);
        
        bool result3 = dfa.match("world");
        print_match(dfa, "world");
        assert(result3 == false);
        
        bool result4 = dfa.match("hello worlds");
        print_match(dfa, "hello worlds");
        assert(result4 == false);
        
        bool result5 = dfa.match("say hello world now");
        print_match(dfa, "say hello world now");
        assert(result5 == false);
    }

    // 单字符测试
    std::cout << "\n=== 单字符测试 ===" << std::endl;
    {
        regex::dfa dfa_single = regex::build_dfa("a");
        bool result6 = dfa_single.match("a");
        print_match(dfa_single, "a");
        assert(result6 == true);
        
        bool result7 = dfa_single.match("b");
        print_match(dfa_single, "b");
        assert(result7 == false);
        
        bool result8 = dfa_single.match("aa");
        print_match(dfa_single, "aa");
        assert(result8 == false);
    }

    // 简单模式测试
    std::cout << "\n=== 简单模式测试 ===" << std::endl;
    {
        regex::dfa dfa_simple = regex::build_dfa("ab");
        bool result9 = dfa_simple.match("ab");
        print_match(dfa_simple, "ab");
        assert(result9 == true);
        
        bool result10 = dfa_simple.match("a");
        print_match(dfa_simple, "a");
        assert(result10 == false);
        
        bool result11 = dfa_simple.match("b");
        print_match(dfa_simple, "b");
        assert(result11 == false);
        
        bool result12 = dfa_simple.match("abc");
        print_match(dfa_simple, "abc");
        assert(result12 == false);
    }

    std::cout << "\n=== 所有调试测试通过! ===" << std::endl;

    return 0;
}
