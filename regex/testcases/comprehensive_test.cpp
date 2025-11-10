#include <iostream>
#include <format>
#include <string_view>
#include <cassert>
#include "regex.hpp"

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
    {
        regex::dfa dfa1 = regex::build_dfa("abc");
        bool result1 = dfa1.match("abc");
        print_match(dfa1, "abc", "abc匹配abc");
        assert(result1 == true);
        
        bool result2 = dfa1.match("ab");
        print_match(dfa1, "ab", "abc匹配ab");
        assert(result2 == false);
        
        bool result3 = dfa1.match("abcd");
        print_match(dfa1, "abcd", "abc匹配abcd");
        assert(result3 == false);
    }

    // 测试2: 单字符匹配
    std::cout << "\n--- 测试2: 单字符 ---" << std::endl;
    {
        regex::dfa dfa2 = regex::build_dfa("x");
        bool result4 = dfa2.match("x");
        print_match(dfa2, "x", "x匹配x");
        assert(result4 == true);
        
        bool result5 = dfa2.match("y");
        print_match(dfa2, "y", "x匹配y");
        assert(result5 == false);
    }

    // 测试3: 特殊字符（如果支持的话）
    std::cout << "\n--- 测试3: 点号通配符 ---" << std::endl;
    {
        regex::dfa dfa3 = regex::build_dfa(".");
        bool result6 = dfa3.match("a");
        print_match(dfa3, "a", ".匹配a");
        assert(result6 == true);
        
        bool result7 = dfa3.match("z");
        print_match(dfa3, "z", ".匹配z");
        assert(result7 == true);
        
        bool result8 = dfa3.match("1");
        print_match(dfa3, "1", ".匹配1");
        assert(result8 == true);
        
        // 额外测试：空字符串应该不匹配单个字符模式
        bool result9 = dfa3.match("");
        std::cout << std::format("{}: {} ({})\n", ".匹配空字符串", result9, "");
        assert(result9 == false);
    }
    
    std::cout << "\n=== 所有综合测试通过! ===" << std::endl;

    return 0;
}
