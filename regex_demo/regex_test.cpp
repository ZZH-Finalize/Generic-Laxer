#include "regex_demo.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "Testing Regex to NFA conversion...\n";
    
    Regex::RegexToNFA converter;
    
    // 测试1: 简单字符匹配
    std::cout << "\nTest 1: Simple character 'a'\n";
    Regex::NFA nfa1 = converter.convert("a");
    converter.print_nfa(nfa1);
    std::cout << "Match 'a': " << Regex::NFA_Matcher::match(nfa1, "a") << std::endl;
    std::cout << "Match 'b': " << Regex::NFA_Matcher::match(nfa1, "b") << std::endl;
    std::cout << "Match 'aa': " << Regex::NFA_Matcher::match(nfa1, "aa") << std::endl;
    
    // 测试2: 连接操作
    std::cout << "\nTest 2: Concatenation 'ab'\n";
    Regex::NFA nfa2 = converter.convert("ab");
    converter.print_nfa(nfa2);
    std::cout << "Match 'ab': " << Regex::NFA_Matcher::match(nfa2, "ab") << std::endl;
    std::cout << "Match 'a': " << Regex::NFA_Matcher::match(nfa2, "a") << std::endl;
    std::cout << "Match 'abc': " << Regex::NFA_Matcher::match(nfa2, "abc") << std::endl;
    
    // 测试3: 选择操作
    std::cout << "\nTest 3: Alternation 'a|b'\n";
    Regex::NFA nfa3 = converter.convert("a|b");
    converter.print_nfa(nfa3);
    std::cout << "Match 'a': " << Regex::NFA_Matcher::match(nfa3, "a") << std::endl;
    std::cout << "Match 'b': " << Regex::NFA_Matcher::match(nfa3, "b") << std::endl;
    std::cout << "Match 'c': " << Regex::NFA_Matcher::match(nfa3, "c") << std::endl;
    std::cout << "Match 'ab': " << Regex::NFA_Matcher::match(nfa3, "ab") << std::endl;
    
    // 测试4: 零次或多次 (Kleene star)
    std::cout << "\nTest 4: Kleene star 'a*'\n";
    Regex::NFA nfa4 = converter.convert("a*");
    converter.print_nfa(nfa4);
    std::cout << "Match '': " << Regex::NFA_Matcher::match(nfa4, "") << std::endl;
    std::cout << "Match 'a': " << Regex::NFA_Matcher::match(nfa4, "a") << std::endl;
    std::cout << "Match 'aa': " << Regex::NFA_Matcher::match(nfa4, "aa") << std::endl;
    std::cout << "Match 'aaa': " << Regex::NFA_Matcher::match(nfa4, "aaa") << std::endl;
    std::cout << "Match 'b': " << Regex::NFA_Matcher::match(nfa4, "b") << std::endl;
    
    // 测试5: 一次或多次
    std::cout << "\nTest 5: One or more 'a+'\n";
    Regex::NFA nfa5 = converter.convert("a+");
    converter.print_nfa(nfa5);
    std::cout << "Match '': " << Regex::NFA_Matcher::match(nfa5, "") << std::endl;
    std::cout << "Match 'a': " << Regex::NFA_Matcher::match(nfa5, "a") << std::endl;
    std::cout << "Match 'aa': " << Regex::NFA_Matcher::match(nfa5, "aa") << std::endl;
    std::cout << "Match 'aaa': " << Regex::NFA_Matcher::match(nfa5, "aaa") << std::endl;
    std::cout << "Match 'b': " << Regex::NFA_Matcher::match(nfa5, "b") << std::endl;
    
    // 测试6: 零次或一次
    std::cout << "\nTest 6: Zero or one 'a?'\n";
    Regex::NFA nfa6 = converter.convert("a?");
    converter.print_nfa(nfa6);
    std::cout << "Match '': " << Regex::NFA_Matcher::match(nfa6, "") << std::endl;
    std::cout << "Match 'a': " << Regex::NFA_Matcher::match(nfa6, "a") << std::endl;
    std::cout << "Match 'aa': " << Regex::NFA_Matcher::match(nfa6, "aa") << std::endl;
    std::cout << "Match 'b': " << Regex::NFA_Matcher::match(nfa6, "b") << std::endl;
    
    // 测试7: 复杂表达式
    std::cout << "\nTest 7: Complex expression '(a|b)*c'\n";
    Regex::NFA nfa7 = converter.convert("(a|b)*c");
    converter.print_nfa(nfa7);
    std::cout << "Match 'c': " << Regex::NFA_Matcher::match(nfa7, "c") << std::endl;
    std::cout << "Match 'ac': " << Regex::NFA_Matcher::match(nfa7, "ac") << std::endl;
    std::cout << "Match 'bc': " << Regex::NFA_Matcher::match(nfa7, "bc") << std::endl;
    std::cout << "Match 'abc': " << Regex::NFA_Matcher::match(nfa7, "abc") << std::endl;
    std::cout << "Match 'abac': " << Regex::NFA_Matcher::match(nfa7, "abac") << std::endl;
    std::cout << "Match 'abab': " << Regex::NFA_Matcher::match(nfa7, "abab") << std::endl;
    
    std::cout << "\nAll tests completed!\n";
    
    return 0;
}
