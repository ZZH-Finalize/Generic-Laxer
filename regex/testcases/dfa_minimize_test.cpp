#include "regex.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "开始DFA最小化测试..." << std::endl;
    
    // 创建一个简单的正则表达式，会生成一个可以被最小化的DFA
    // 例如: (a|b)*a(a|b) - 匹配以'a'为倒数第二个字符的字符串
    
    try {
        auto dfa = regex::build_dfa("(a|b)*a(a|b)");
        
        std::cout << "原始DFA状态数: " << dfa.get_state_count() << std::endl;
        
        // 测试原始DFA并添加断言
        auto result1 = dfa.match("aa");
        std::cout << "原始DFA匹配 'aa': " << result1.has_value() << std::endl;
        assert(result1.has_value());
        
        auto result2 = dfa.match("ab");
        std::cout << "原始DFA匹配 'ab': " << result2.has_value() << std::endl;
        assert(result2.has_value());
        
        auto result3 = dfa.match("ba");
        std::cout << "原始DFA匹配 'ba': " << result3.has_value() << std::endl;
        assert(!result3.has_value());
        
        auto result4 = dfa.match("bb");
        std::cout << "原始DFA匹配 'bb': " << result4.has_value() << std::endl;
        assert(!result4.has_value());
        
        auto result5 = dfa.match("aaa");
        std::cout << "原始DFA匹配 'aaa': " << result5.has_value() << std::endl;
        assert(result5.has_value());
        
        auto result6 = dfa.match("aba");
        std::cout << "原始DFA匹配 'aba': " << result6.has_value() << std::endl;
        assert(!result6.has_value());
        
        auto result7 = dfa.match("aab");
        std::cout << "原始DFA匹配 'aab': " << result7.has_value() << std::endl;
        assert(result7.has_value());
        
        // 最小化DFA
        dfa = regex::minimize(dfa);
        
        std::cout << "最小化后DFA状态数: " << dfa.get_state_count() << std::endl;
        
        // 测试最小化后的DFA，结果应该相同
        auto result8 = dfa.match("aa");
        std::cout << "最小化DFA匹配 'aa': " << result8.has_value() << std::endl;
        assert(result8.has_value());
        
        auto result9 = dfa.match("ab");
        std::cout << "最小化DFA匹配 'ab': " << result9.has_value() << std::endl;
        assert(result9.has_value());
        
        auto result10 = dfa.match("ba");
        std::cout << "最小化DFA匹配 'ba': " << result10.has_value() << std::endl;
        assert(!result10.has_value());
        
        auto result11 = dfa.match("bb");
        std::cout << "最小化DFA匹配 'bb': " << result11.has_value() << std::endl;
        assert(!result11.has_value());
        
        auto result12 = dfa.match("aaa");
        std::cout << "最小化DFA匹配 'aaa': " << result12.has_value() << std::endl;
        assert(result12.has_value());
        
        auto result13 = dfa.match("aba");
        std::cout << "最小化DFA匹配 'aba': " << result13.has_value() << std::endl;
        assert(!result13.has_value());
        
        auto result14 = dfa.match("aab");
        std::cout << "最小化DFA匹配 'aab': " << result14.has_value() << std::endl;
        assert(result14.has_value());
        
        std::cout << "DFA最小化测试通过!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
