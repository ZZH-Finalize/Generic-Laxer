#include <iostream>
#include <format>
#include <string_view>
#include <cassert>
#include "regex.hpp"

void test_nfa_merge()
{
    std::cout << "=== 测试NFA直接合并功能 ===" << std::endl;

    // 创建独立的NFA对象（直接通过NFA构建，而不是通过正则表达式）
    regex::nfa keyword_nfa    = regex::build_nfa("if");
    regex::nfa identifier_nfa = regex::build_nfa("variable");
    regex::nfa number_nfa     = regex::build_nfa("123");

    std::cout << "原始关键字NFA: " << std::format("{}", keyword_nfa) << std::endl;
    std::cout << "原始标识符NFA: " << std::format("{}", identifier_nfa) << std::endl;
    std::cout << "原始数字NFA: " << std::format("{}", number_nfa) << std::endl;

    // 使用新的 | 操作符合并NFA
    regex::nfa combined_nfa = keyword_nfa | identifier_nfa;
    std::cout << "关键字|标识符 NFA: " << std::format("{}", combined_nfa) << std::endl;

    // 再合并第三个NFA
    regex::nfa final_nfa = combined_nfa | number_nfa;
    std::cout << "最终合并NFA (关键字|标识符|数字): " << std::format("{}", final_nfa)
              << std::endl;

    // 将合并后的NFA转换为DFA
    regex::dfa combined_dfa = regex::to_dfa(final_nfa);

    // 测试合并后的DFA并添加断言
    std::cout << "\n--- 测试合并后的DFA ---" << std::endl;
    bool result1 = combined_dfa.match("if");
    std::cout << "匹配 'if': " << result1 << std::endl;
    assert(result1 == true);

    bool result2 = combined_dfa.match("variable");
    std::cout << "匹配 'variable': " << result2 << std::endl;
    assert(result2 == true);

    bool result3 = combined_dfa.match("123");
    std::cout << "匹配 '123': " << result3 << std::endl;
    assert(result3 == true);

    bool result4 = combined_dfa.match("else"); // 不应该匹配
    std::cout << "匹配 'else': " << result4 << std::endl;
    assert(result4 == false);

    bool result5 = combined_dfa.match("456"); // 不应该匹配
    std::cout << "匹配 '456': " << result5 << std::endl;
    assert(result5 == false);

    // 测试更复杂的合并
    std::cout << "\n--- 测试复杂合并 ---" << std::endl;
    regex::nfa abc_nfa = regex::build_nfa("abc");
    regex::nfa xyz_nfa = regex::build_nfa("xyz");
    regex::nfa num_nfa = regex::build_nfa("[0-9]+");

    // 合并多个NFA
    regex::nfa complex_nfa = abc_nfa | xyz_nfa;
    complex_nfa            = complex_nfa | num_nfa;

    regex::dfa complex_dfa = regex::to_dfa(complex_nfa);

    std::cout << "复杂合并DFA测试:" << std::endl;
    bool result6 = complex_dfa.match("abc");
    std::cout << "匹配 'abc': " << result6 << std::endl;
    assert(result6 == true);

    bool result7 = complex_dfa.match("xyz");
    std::cout << "匹配 'xyz': " << result7 << std::endl;
    assert(result7 == true);

    bool result8 = complex_dfa.match("789");
    std::cout << "匹配 '789': " << result8 << std::endl;
    assert(result8 == true);

    bool result9 = complex_dfa.match("hello");
    std::cout << "匹配 'hello': " << result9 << std::endl;
    assert(result9 == false);

    // 验证每个单独的NFA仍然有效
    std::cout << "\n--- 验证原NFA未被修改 ---" << std::endl;
    regex::dfa original_keyword_dfa    = regex::to_dfa(keyword_nfa);
    regex::dfa original_identifier_dfa = regex::to_dfa(identifier_nfa);
    regex::dfa original_number_dfa     = regex::to_dfa(number_nfa);

    bool result10 = original_keyword_dfa.match("if");
    std::cout << "原关键字NFA匹配'if': " << result10 << std::endl;
    assert(result10 == true);

    bool result11 = original_keyword_dfa.match("variable");
    std::cout << "原关键字NFA匹配'variable': " << result11 << std::endl;
    assert(result11 == false);

    bool result12 = original_identifier_dfa.match("variable");
    std::cout << "原标识符NFA匹配'variable': " << result12 << std::endl;
    assert(result12 == true);

    bool result13 = original_identifier_dfa.match("if");
    std::cout << "原标识符NFA匹配'if': " << result13 << std::endl;
    assert(result13 == false);

    bool result14 = original_number_dfa.match("123");
    std::cout << "原数字NFA匹配'123': " << result14 << std::endl;
    assert(result14 == true);

    bool result15 = original_number_dfa.match("456");
    std::cout << "原数字NFA匹配'456': " << result15 << std::endl;
    assert(result15 == false);
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
    regex::dfa left_dfa   = regex::to_dfa(left_assoc);

    // 测试 a | (b | c)
    regex::nfa right_assoc = a_nfa | (b_nfa | c_nfa);
    regex::dfa right_dfa   = regex::to_dfa(right_assoc);

    // 测试左侧结合的结果
    bool left_result1 = left_dfa.match("a");
    std::cout << "左侧结合 (a|b)|c 匹配 'a': " << left_result1 << std::endl;
    assert(left_result1 == true);

    bool left_result2 = left_dfa.match("b");
    std::cout << "左侧结合 (a|b)|c 匹配 'b': " << left_result2 << std::endl;
    assert(left_result2 == true);

    bool left_result3 = left_dfa.match("c");
    std::cout << "左侧结合 (a|b)|c 匹配 'c': " << left_result3 << std::endl;
    assert(left_result3 == true);

    bool left_result4 = left_dfa.match("d");
    std::cout << "左侧结合 (a|b)|c 匹配 'd': " << left_result4 << std::endl;
    assert(left_result4 == false);

    // 测试右侧结合的结果
    bool right_result1 = right_dfa.match("a");
    std::cout << "右侧结合 a|(b|c) 匹配 'a': " << right_result1 << std::endl;
    assert(right_result1 == true);

    bool right_result2 = right_dfa.match("b");
    std::cout << "右侧结合 a|(b|c) 匹配 'b': " << right_result2 << std::endl;
    assert(right_result2 == true);

    bool right_result3 = right_dfa.match("c");
    std::cout << "右侧结合 a|(b|c) 匹配 'c': " << right_result3 << std::endl;
    assert(right_result3 == true);

    bool right_result4 = right_dfa.match("d");
    std::cout << "右侧结合 a|(b|c) 匹配 'd': " << right_result4 << std::endl;
    assert(right_result4 == false);
}

int main()
{
    test_nfa_merge();
    test_merge_associativity();

    std::cout << "\n=== 所有NFA直接合并测试通过 ===" << std::endl;
    std::cout << "结论：NFA类现在支持直接合并操作，可以实现类似Flex的多规则并行匹配。"
              << std::endl;

    return 0;
}
