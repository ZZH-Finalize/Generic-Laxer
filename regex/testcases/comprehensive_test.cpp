#include <iostream>
#include <format>
#include <string_view>
#include <cassert>
#include "regex.hpp"

void print_match(const auto& dfa, const std::string_view& str, const std::string& test_name)
{
    auto result = dfa.match(str);
    bool has_match = result.has_value();
    std::cout << std::format("{}: {} ({})\n", test_name, has_match, str);
}

int main(const int argc, const char** argv)
{
    std::cout << "=== 综合测试开始 ===" << std::endl;

    // 测试1: 简单字符串匹配
    std::cout << "\n--- 测试1: 简单字符串 ---" << std::endl;
    {
        auto dfa1 = regex::build_dfa("abc");
        auto result1 = dfa1.match("abc");
        print_match(dfa1, "abc", "abc匹配abc");
        assert(result1.has_value());
        
        auto result2 = dfa1.match("ab");
        print_match(dfa1, "ab", "abc匹配ab");
        assert(!result2.has_value());
        
        auto result3 = dfa1.match("abcd");
        print_match(dfa1, "abcd", "abc匹配abcd");
        assert(!result3.has_value());
    }

    // 测试2: 单字符匹配
    std::cout << "\n--- 测试2: 单字符 ---" << std::endl;
    {
        auto dfa2 = regex::build_dfa("x");
        auto result4 = dfa2.match("x");
        print_match(dfa2, "x", "x匹配x");
        assert(result4.has_value());
        
        auto result5 = dfa2.match("y");
        print_match(dfa2, "y", "x匹配y");
        assert(!result5.has_value());
    }

    // 测试3: 特殊字符（如果支持的话）
    std::cout << "\n--- 测试3: 点号通配符 ---" << std::endl;
    {
        auto dfa3 = regex::build_dfa(".");
        auto result6 = dfa3.match("a");
        print_match(dfa3, "a", ".匹配a");
        assert(result6.has_value());
        
        auto result7 = dfa3.match("z");
        print_match(dfa3, "z", ".匹配z");
        assert(result7.has_value());
        
        auto result8 = dfa3.match("1");
        print_match(dfa3, "1", ".匹配1");
        assert(result8.has_value());
        
        // 额外测试：空字符串应该不匹配单个字符模式
        auto result9 = dfa3.match("");
        bool has_match9 = result9.has_value();
        std::cout << std::format("{}: {} ({})\n", ".匹配空字符串", has_match9, "");
        assert(!has_match9);
    }
    
    std::cout << "\n=== 所有综合测试通过! ===" << std::endl;

    return 0;
}
