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
    std::cout << "开始基本测试..." << std::endl;
    
    regex::dfa dfa = regex::build_dfa("hello world");

    // 添加断言来验证匹配结果
    assert(dfa.match("hello world") == true);
    std::cout << "测试 'hello world': " << dfa.match("hello world") << std::endl;
    
    // 添加其他测试用例来验证结果
    assert(dfa.match("hello") == false);
    std::cout << "测试 'hello': " << dfa.match("hello") << std::endl;
    
    assert(dfa.match("world") == false);
    std::cout << "测试 'world': " << dfa.match("world") << std::endl;
    
    assert(dfa.match("hello world!") == false);
    std::cout << "测试 'hello world!': " << dfa.match("hello world!") << std::endl;

    std::cout << "基本测试通过!" << std::endl;

    return 0;
}
