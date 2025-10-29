#include <iostream>
#include <format>
#include <string_view>
#include "regex/regex.hpp"

int main(const int argc, const char** argv)
{
    std::cout << "测试NFA构造..." << std::endl;
    
    try {
        // 先测试NFA构造
        regex::nfa nfa_obj = regex::build_nfa("a");
        std::cout << "NFA构造成功: " << nfa_obj.to_string() << std::endl;
        
        std::cout << "测试DFA构造..." << std::endl;
        // 测试DFA构造
        regex::dfa dfa_obj = regex::build_dfa(nfa_obj);
        std::cout << "DFA构造成功" << std::endl;
        
        // 测试匹配
        std::cout << "测试匹配 'a': " << dfa_obj.match("a") << std::endl;
        std::cout << "测试匹配 'b': " << dfa_obj.match("b") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "异常: " << e.what() << std::endl;
    }

    return 0;
}
