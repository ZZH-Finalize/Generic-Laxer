#include <iostream>
#include <format>
#include <string_view>
#include <cassert>
#include "regex.hpp"

void print_match(const auto& dfa, const std::string_view& str)
{
    auto result = dfa.match(str);
    std::cout << std::format("{}: {}\n", str, result.has_value());
}

int main(const int argc, const char** argv)
{
    std::cout << "开始基本测试..." << std::endl;
    
    auto dfa = regex::build_dfa("hello world");

    // 添加断言来验证匹配结果
    assert(dfa.match("hello world").has_value());
    std::cout << "测试 'hello world': " << dfa.match("hello world").has_value() << std::endl;
    
    // 添加其他测试用例来验证结果
    assert(!dfa.match("hello").has_value());
    std::cout << "测试 'hello': " << dfa.match("hello").has_value() << std::endl;
    
    assert(!dfa.match("world").has_value());
    std::cout << "测试 'world': " << dfa.match("world").has_value() << std::endl;
    
    assert(!dfa.match("hello world!").has_value());
    std::cout << "测试 'hello world!': " << dfa.match("hello world!").has_value() << std::endl;

    std::cout << "基本测试通过!" << std::endl;

    return 0;
}
