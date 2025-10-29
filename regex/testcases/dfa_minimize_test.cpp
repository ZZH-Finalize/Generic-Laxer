#include "regex.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "开始DFA最小化测试..." << std::endl;
    
    // 创建一个简单的正则表达式，会生成一个可以被最小化的DFA
    // 例如: (a|b)*a(a|b) - 匹配以'a'为倒数第二个字符的字符串
    
    try {
        regex::dfa dfa = regex::build_dfa("(a|b)*a(a|b)");
        
        std::cout << "原始DFA状态数: " << dfa.get_state_count() << std::endl;
        
        // 测试原始DFA并添加断言
        bool result1 = dfa.match("aa");
        std::cout << "原始DFA匹配 'aa': " << result1 << std::endl;
        assert(result1 == true);
        
        bool result2 = dfa.match("ab");
        std::cout << "原始DFA匹配 'ab': " << result2 << std::endl;
        assert(result2 == true);
        
        bool result3 = dfa.match("ba");
        std::cout << "原始DFA匹配 'ba': " << result3 << std::endl;
        assert(result3 == false);
        
        bool result4 = dfa.match("bb");
        std::cout << "原始DFA匹配 'bb': " << result4 << std::endl;
        assert(result4 == false);
        
        bool result5 = dfa.match("aaa");
        std::cout << "原始DFA匹配 'aaa': " << result5 << std::endl;
        assert(result5 == true);
        
        bool result6 = dfa.match("aba");
        std::cout << "原始DFA匹配 'aba': " << result6 << std::endl;
        assert(result6 == false);
        
        bool result7 = dfa.match("aab");
        std::cout << "原始DFA匹配 'aab': " << result7 << std::endl;
        assert(result7 == true);
        
        // 最小化DFA
        dfa = regex::minimize(dfa);
        
        std::cout << "最小化后DFA状态数: " << dfa.get_state_count() << std::endl;
        
        // 测试最小化后的DFA，结果应该相同
        bool result8 = dfa.match("aa");
        std::cout << "最小化DFA匹配 'aa': " << result8 << std::endl;
        assert(result8 == true);
        
        bool result9 = dfa.match("ab");
        std::cout << "最小化DFA匹配 'ab': " << result9 << std::endl;
        assert(result9 == true);
        
        bool result10 = dfa.match("ba");
        std::cout << "最小化DFA匹配 'ba': " << result10 << std::endl;
        assert(result10 == false);
        
        bool result11 = dfa.match("bb");
        std::cout << "最小化DFA匹配 'bb': " << result11 << std::endl;
        assert(result11 == false);
        
        bool result12 = dfa.match("aaa");
        std::cout << "最小化DFA匹配 'aaa': " << result12 << std::endl;
        assert(result12 == true);
        
        bool result13 = dfa.match("aba");
        std::cout << "最小化DFA匹配 'aba': " << result13 << std::endl;
        assert(result13 == false);
        
        bool result14 = dfa.match("aab");
        std::cout << "最小化DFA匹配 'aab': " << result14 << std::endl;
        assert(result14 == true);
        
        std::cout << "DFA最小化测试通过!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
