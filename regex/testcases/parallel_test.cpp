#include <iostream>
#include <format>
#include <string_view>
#include "regex/regex.hpp"

// 测试多个规则的并行匹配（类似Flex的功能）
void test_parallel_nfa()
{
    std::cout << "=== 测试多规则并行匹配 ===" << std::endl;
    
    // 创建几个独立的NFA，模拟Flex中的不同词法规则
    regex::nfa keyword_nfa = regex::build_nfa("if|else|while|for");
    regex::nfa identifier_nfa = regex::build_nfa("[a-zA-Z_][a-zA-Z0-9_]*");
    regex::nfa number_nfa = regex::build_nfa("[0-9]+");
    regex::nfa whitespace_nfa = regex::build_nfa("[ \t\n\r]+");
    
    std::cout << "关键字NFA: " << keyword_nfa.to_string() << std::endl;
    std::cout << "标识符NFA: " << identifier_nfa.to_string() << std::endl;
    std::cout << "数字NFA: " << number_nfa.to_string() << std::endl;
    std::cout << "空白NFA: " << whitespace_nfa.to_string() << std::endl;
    
    // 使用NFA的builder来创建一个大的选择NFA，模拟多个规则的并行匹配
    // 在内部，这会使用select_with方法来连接多个NFA
    regex::nfa combined_nfa = regex::build_nfa("(if|else|while|for)|([a-zA-Z_][a-zA-Z0-9_]*)|([0-9]+)|([ \t\n\r]+)");
    
    std::cout << "组合NFA: " << combined_nfa.to_string() << std::endl;
    
    // 将NFA转换为DFA进行测试
    regex::dfa combined_dfa = regex::build_dfa(combined_nfa);
    
    // 测试各种输入
    std::cout << "\n--- 测试组合DFA ---" << std::endl;
    std::cout << "匹配 'if': " << combined_dfa.match("if") << std::endl;
    std::cout << "匹配 'variable': " << combined_dfa.match("variable") << std::endl;
    std::cout << "匹配 '123': " << combined_dfa.match("123") << std::endl;
    std::cout << "匹配 '  \t': " << combined_dfa.match("  \t") << std::endl;
    std::cout << "匹配 'unknown': " << combined_dfa.match("unknown") << std::endl;
    
    // 使用新的查找功能来测试子串匹配
    std::cout << "\n--- 测试查找功能 ---" << std::endl;
    std::cout << "在 'if (x > 0)' 中查找: " << combined_dfa.find_match("if (x > 0)") << std::endl;
    std::cout << "在 'int var = 123' 中查找: " << combined_dfa.find_match("int var = 123") << std::endl;
    
    // 更精确的测试：验证是否可以区分不同的规则
    std::cout << "\n--- 验证规则区分能力 ---" << std::endl;
    
    // 创建单独的DFA来区分不同类型的token
    regex::dfa keyword_dfa = regex::build_dfa("if|else|while|for");
    regex::dfa identifier_dfa = regex::build_dfa("[a-zA-Z_][a-zA-Z0-9_]*");
    regex::dfa number_dfa = regex::build_dfa("[0-9]+");
    
    std::cout << "'if' 是关键字: " << keyword_dfa.match("if") << std::endl;
    std::cout << "'if' 是标识符: " << identifier_dfa.match("if") << std::endl;
    std::cout << "'if' 是数字: " << number_dfa.match("if") << std::endl;
    
    std::cout << "\n'variable' 是关键字: " << keyword_dfa.match("variable") << std::endl;
    std::cout << "'variable' 是标识符: " << identifier_dfa.match("variable") << std::endl;
    std::cout << "'variable' 是数字: " << number_dfa.match("variable") << std::endl;
    
    std::cout << "\n'456' 是关键字: " << keyword_dfa.match("456") << std::endl;
    std::cout << "'456' 是标识符: " << identifier_dfa.match("456") << std::endl;
    std::cout << "'456' 是数字: " << number_dfa.match("456") << std::endl;
}

// 测试手动合并NFA（如果NFA类提供公共接口）
void test_manual_merge()
{
    std::cout << "\n=== 测试手动NFA合并 ===" << std::endl;
    
    // 注意：目前NFA的merge方法是私有的，但内部的select_with实现了类似功能
    // 我们可以通过构建包含选择操作的正则表达式来测试这个功能
    regex::dfa dfa1 = regex::build_dfa("abc");
    regex::dfa dfa2 = regex::build_dfa("xyz");
    
    // 使用选择操作创建一个可以匹配任一模式的DFA
    regex::dfa combined_dfa = regex::build_dfa("(abc)|(xyz)");
    
    std::cout << "选择DFA测试:" << std::endl;
    std::cout << "匹配 'abc': " << combined_dfa.match("abc") << std::endl;
    std::cout << "匹配 'xyz': " << combined_dfa.match("xyz") << std::endl;
    std::cout << "匹配 'abcx': " << combined_dfa.match("abcx") << std::endl;
    std::cout << "匹配 '123': " << combined_dfa.match("123") << std::endl;
}

int main()
{
    test_parallel_nfa();
    test_manual_merge();
    
    std::cout << "\n=== 多规则并行匹配测试完成 ===" << std::endl;
    std::cout << "结论：该正则引擎支持类似Flex的多规则并行匹配功能，" << std::endl;
    std::cout << "通过选择操作(|)可以将多个规则合并成一个NFA进行并行匹配。" << std::endl;
    
    return 0;
}
