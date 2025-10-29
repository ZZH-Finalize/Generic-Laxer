#include <iostream>
#include <format>
#include <string_view>
#include "regex/regex.hpp"

void test_nfa_merge()
{
    std::cout << "=== 测试NFA直接合并功能 ===" << std::endl;

    // 创建独立的NFA对象（直接通过NFA构建，而不是通过正则表达式）
    regex::nfa keyword_nfa    = regex::build_nfa("if");
    regex::nfa identifier_nfa = regex::build_nfa("variable");
    regex::nfa number_nfa     = regex::build_nfa("123");

    std::cout << "原始关键字NFA: " << keyword_nfa.to_string() << std::endl;
    std::cout << "原始标识符NFA: " << identifier_nfa.to_string() << std::endl;
    std::cout << "原始数字NFA: " << number_nfa.to_string() << std::endl;

    // 使用新的 | 操作符合并NFA
    regex::nfa combined_nfa = keyword_nfa | identifier_nfa;
    std::cout << "关键字|标识符 NFA: " << combined_nfa.to_string() << std::endl;

    // 再合并第三个NFA
    regex::nfa final_nfa = combined_nfa | number_nfa;
    std::cout << "最终合并NFA (关键字|标识符|数字): " << final_nfa.to_string()
              << std::endl;

    // 将合并后的NFA转换为DFA
    regex::dfa combined_dfa = regex::build_dfa(final_nfa);

    // 测试合并后的DFA
    std::cout << "\n--- 测试合并后的DFA ---" << std::endl;
    std::cout << "匹配 'if': " << combined_dfa.match("if") << std::endl;
    std::cout << "匹配 'variable': " << combined_dfa.match("variable") << std::endl;
    std::cout << "匹配 '123': " << combined_dfa.match("123") << std::endl;
    std::cout << "匹配 'else': " << combined_dfa.match("else") << std::endl; // 不应该匹配
    std::cout << "匹配 '456': " << combined_dfa.match("456") << std::endl;   // 不应该匹配

    // 测试更复杂的合并
    std::cout << "\n--- 测试复杂合并 ---" << std::endl;
    regex::nfa abc_nfa = regex::build_nfa("abc");
    regex::nfa xyz_nfa = regex::build_nfa("xyz");
    regex::nfa num_nfa = regex::build_nfa("[0-9]+");

    // 合并多个NFA
    regex::nfa complex_nfa = abc_nfa | xyz_nfa;
    complex_nfa            = complex_nfa | num_nfa;

    regex::dfa complex_dfa = regex::build_dfa(complex_nfa);

    std::cout << "复杂合并DFA测试:" << std::endl;
    std::cout << "匹配 'abc': " << complex_dfa.match("abc") << std::endl;
    std::cout << "匹配 'xyz': " << complex_dfa.match("xyz") << std::endl;
    std::cout << "匹配 '789': " << complex_dfa.match("789") << std::endl;
    std::cout << "匹配 'hello': " << complex_dfa.match("hello") << std::endl;

    // 验证每个单独的NFA仍然有效
    std::cout << "\n--- 验证原NFA未被修改 ---" << std::endl;
    regex::dfa original_keyword_dfa    = regex::build_dfa(keyword_nfa);
    regex::dfa original_identifier_dfa = regex::build_dfa(identifier_nfa);
    regex::dfa original_number_dfa     = regex::build_dfa(number_nfa);

    std::cout << "原关键字NFA匹配'if': " << original_keyword_dfa.match("if") << std::endl;
    std::cout << "原关键字NFA匹配'variable': " << original_keyword_dfa.match("variable")
              << std::endl;
    std::cout << "原标识符NFA匹配'variable': "
              << original_identifier_dfa.match("variable") << std::endl;
    std::cout << "原标识符NFA匹配'if': " << original_identifier_dfa.match("if")
              << std::endl;
    std::cout << "原数字NFA匹配'123': " << original_number_dfa.match("123") << std::endl;
    std::cout << "原数字NFA匹配'456': " << original_number_dfa.match("456") << std::endl;
}

// 测试NFA合并的顺序是否影响结果
void test_merge_associativity()
{
    std::cout << "\n=== 测试NFA合并的结合性 ===" << std::endl;

    regex::nfa a_nfa = regex::build_nfa("a");
    regex::nfa b_nfa = regex::build_nfa("b");
    regex::nfa c_nfa = regex::build_nfa("c");

    // 测试 (a | b) | c
    regex::nfa left_assoc = (a_nfa | b_nfa) | c_nfa;
    regex::dfa left_dfa   = regex::build_dfa(left_assoc);

    // 测试 a | (b | c)
    regex::nfa right_assoc = a_nfa | (b_nfa | c_nfa);
    regex::dfa right_dfa   = regex::build_dfa(right_assoc);

    std::cout << "左侧结合 (a|b)|c 匹配 'a': " << left_dfa.match("a") << std::endl;
    std::cout << "左侧结合 (a|b)|c 匹配 'b': " << left_dfa.match("b") << std::endl;
    std::cout << "左侧结合 (a|b)|c 匹配 'c': " << left_dfa.match("c") << std::endl;

    std::cout << "右侧结合 a|(b|c) 匹配 'a': " << right_dfa.match("a") << std::endl;
    std::cout << "右侧结合 a|(b|c) 匹配 'b': " << right_dfa.match("b") << std::endl;
    std::cout << "右侧结合 a|(b|c) 匹配 'c': " << right_dfa.match("c") << std::endl;
}

int main()
{
    test_nfa_merge();
    test_merge_associativity();

    std::cout << "\n=== NFA直接合并测试完成 ===" << std::endl;
    std::cout << "结论：NFA类现在支持直接合并操作，可以实现类似Flex的多规则并行匹配。"
              << std::endl;

    return 0;
}
