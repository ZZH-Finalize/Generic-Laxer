#include "regex_demo.hpp"
#include <iostream>

int main() {
    std::cout << "Simple test for regex to NFA conversion\n";
    
    Regex::RegexToNFA converter;
    
    // 测试简单的 'a' 匹配
    std::cout << "\nTesting 'a':\n";
    Regex::NFA nfa = converter.convert("a");
    converter.print_nfa(nfa);
    
    std::cout << "Testing match 'a': " << Regex::NFA_Matcher::match(nfa, "a") << std::endl;
    std::cout << "Testing match 'b': " << Regex::NFA_Matcher::match(nfa, "b") << std::endl;
    std::cout << "Testing match '': " << Regex::NFA_Matcher::match(nfa, "") << std::endl;
    
    return 0;
}
