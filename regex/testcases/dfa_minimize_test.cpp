#include "dfa.hpp"
#include <iostream>

int main() {
    // 创建一个简单的正则表达式，会生成一个可以被最小化的DFA
    // 例如: (a|b)*a(a|b) - 匹配以'a'为倒数第二个字符的字符串
    
    try {
        regex::dfa dfa = regex::dfa::from("(a|b)*a(a|b)");
        
        std::cout << "原始DFA状态数: " << dfa.get_state_count() << std::endl;
        
        // 测试原始DFA
        std::cout << "原始DFA匹配 'aa': " << dfa.match("aa") << std::endl;
        std::cout << "原始DFA匹配 'ab': " << dfa.match("ab") << std::endl;
        std::cout << "原始DFA匹配 'ba': " << dfa.match("ba") << std::endl;
        std::cout << "原始DFA匹配 'bb': " << dfa.match("bb") << std::endl;
        std::cout << "原始DFA匹配 'aaa': " << dfa.match("aaa") << std::endl;
        std::cout << "原始DFA匹配 'aba': " << dfa.match("aba") << std::endl;
        std::cout << "原始DFA匹配 'aab': " << dfa.match("aab") << std::endl;
        
        // 最小化DFA
        dfa.minimize();
        
        std::cout << "最小化后DFA状态数: " << dfa.get_state_count() << std::endl;
        
        // 测试最小化后的DFA
        std::cout << "最小化DFA匹配 'aa': " << dfa.match("aa") << std::endl;
        std::cout << "最小化DFA匹配 'ab': " << dfa.match("ab") << std::endl;
        std::cout << "最小化DFA匹配 'ba': " << dfa.match("ba") << std::endl;
        std::cout << "最小化DFA匹配 'bb': " << dfa.match("bb") << std::endl;
        std::cout << "最小化DFA匹配 'aaa': " << dfa.match("aaa") << std::endl;
        std::cout << "最小化DFA匹配 'aba': " << dfa.match("aba") << std::endl;
        std::cout << "最小化DFA匹配 'aab': " << dfa.match("aab") << std::endl;
        
        std::cout << "DFA最小化测试完成!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
