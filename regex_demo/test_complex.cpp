#include "regex_demo.hpp"
#include <iostream>

int main() {
    std::cout << "Testing complex regex patterns\n";
    
    Regex::RegexToNFA converter;
    
    // 测试 'a|b'
    std::cout << "\nTesting 'a|b':\n";
    Regex::NFA nfa = converter.convert("a|b");
    converter.print_nfa(nfa);
    
    std::cout << "Testing match 'a': " << Regex::NFA_Matcher::match(nfa, "a") << std::endl;
    std::cout << "Testing match 'b': " << Regex::NFA_Matcher::match(nfa, "b") << std::endl;
    std::cout << "Testing match 'ab': " << Regex::NFA_Matcher::match(nfa, "ab") << std::endl;
    std::cout << "Testing match 'c': " << Regex::NFA_Matcher::match(nfa, "c") << std::endl;
    
    // 测试 'ab'
    std::cout << "\nTesting 'ab':\n";
    Regex::NFA nfa2 = converter.convert("ab");
    converter.print_nfa(nfa2);
    
    std::cout << "Testing match 'ab': " << Regex::NFA_Matcher::match(nfa2, "ab") << std::endl;
    std::cout << "Testing match 'a': " << Regex::NFA_Matcher::match(nfa2, "a") << std::endl;
    std::cout << "Testing match 'b': " << Regex::NFA_Matcher::match(nfa2, "b") << std::endl;
    
    return 0;
}
