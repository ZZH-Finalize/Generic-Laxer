#include "regex_demo.hpp"
#include <format>
#include <functional>
#include <iostream>
#include <unordered_map>

int main()
{
    std::cout << "Testing NFA to DFA conversion\n";

    Regex::RegexToNFA converter;

    // 测试 'a|b' 转换为DFA
    std::cout << "\nTesting 'a|b':\n";
    Regex::NFA nfa = converter.convert("a|b");
    std::cout << "NFA created. Converting to DFA...\n";

    Regex::DFA dfa = Regex::NFAtoDFAConverter::convert(nfa);
    std::cout << "DFA created with " << dfa.states.size() << " states\n";

    // 测试DFA匹配
    std::cout << "Testing DFA match 'a': " << Regex::DFA_Matcher::match(dfa, "a")
              << std::endl;
    std::cout << "Testing DFA match 'b': " << Regex::DFA_Matcher::match(dfa, "b")
              << std::endl;
    std::cout << "Testing DFA match 'ab': " << Regex::DFA_Matcher::match(dfa, "ab")
              << std::endl;
    std::cout << "Testing DFA match 'c': " << Regex::DFA_Matcher::match(dfa, "c")
              << std::endl;

    // 测试 'a*' 转换为DFA
    std::cout << "\nTesting 'a*':\n";
    Regex::NFA nfa2 = converter.convert("a*");
    std::cout << "NFA created. Converting to DFA...\n";

    Regex::DFA dfa2 = Regex::NFAtoDFAConverter::convert(nfa2);
    std::cout << "DFA created with " << dfa2.states.size() << " states\n";

    // 测试DFA匹配
    std::cout << "Testing DFA match '': " << Regex::DFA_Matcher::match(dfa2, "")
              << std::endl;
    std::cout << "Testing DFA match 'a': " << Regex::DFA_Matcher::match(dfa2, "a")
              << std::endl;
    std::cout << "Testing DFA match 'aa': " << Regex::DFA_Matcher::match(dfa2, "aa")
              << std::endl;
    std::cout << "Testing DFA match 'aaa': " << Regex::DFA_Matcher::match(dfa2, "aaa")
              << std::endl;

    // 比较NFA和DFA的匹配结果
    std::cout << "\nComparing NFA and DFA results for 'a|b':\n";
    std::cout << "NFA match 'a': " << Regex::NFA_Matcher::match(nfa, "a") << std::endl;
    std::cout << "DFA match 'a': " << Regex::DFA_Matcher::match(dfa, "a") << std::endl;
    std::cout << "NFA match 'b': " << Regex::NFA_Matcher::match(nfa, "b") << std::endl;
    std::cout << "DFA match 'b': " << Regex::DFA_Matcher::match(dfa, "b") << std::endl;

    // 测试复杂表达式
    std::cout << "test complex regex" << std::endl;
    // Regex::NFA nfa_complex = converter.convert("[a-z]+");
    Regex::NFA nfa_complex2 = converter.convert("0x[0-9a-fA-F]+");

    std::cout << std::format("nfa states: {}\n", nfa_complex2.states.size());
    std::cout << std::format("nfa start: {}\n", nfa_complex2.start_state);
    std::cout << std::format("nfa final: {}\n", nfa_complex2.final_state);

    std::hash<int> hash_fn_int;
    std::hash<char> hash_fn_char;

    for (auto i : {1, 2, 3, 4}) {
        std::cout << std::format("std::hash({}) -> {}\n", i, hash_fn_int(i));
    }

    std::unordered_map<char, int> map;

    for (auto i : {'a', '2', 'c', 'd'}) {
        std::cout << std::format("std::hash({}) -> {}\n", (int) i, hash_fn_char(i));
    }

    return 0;
}
