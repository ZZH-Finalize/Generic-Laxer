#include <iostream>
#include <format>
#include <string_view>
#include <cassert>
#include "regex.hpp"

// 测试多个规则的并行匹配（类似Flex的功能）
void test_parallel_nfa()
{
    std::cout << "=== 测试多规则并行匹配 ===" << std::endl;
    
    // 创建几个独立的NFA，模拟Flex中的不同词法规则
    regex::nfa keyword_nfa = regex::build_nfa("if|else|while|for");
    regex::nfa identifier_nfa = regex::build_nfa("[a-zA-Z_][a-zA-Z0-9_]*");
    regex::nfa number_nfa = regex::build_nfa("[0-9]+");
    regex::nfa whitespace_nfa = regex::build_nfa("[ \t\n\r]+");
    
    std::cout << "关键字NFA: " << std::format("{}", keyword_nfa) << std::endl;
    std::cout << "标识符NFA: " << std::format("{}", identifier_nfa) << std::endl;
    std::cout << "数字NFA: " << std::format("{}", number_nfa) << std::endl;
    std::cout << "空白NFA: " << std::format("{}", whitespace_nfa) << std::endl;
    
    // 使用NFA的builder来创建一个大的选择NFA，模拟多个规则的并行匹配
    // 在内部，这会使用select_with方法来连接多个NFA
    regex::nfa combined_nfa = regex::build_nfa("(if|else|while|for)|([a-zA-Z_][a-zA-Z0-9_]*)|([0-9]+)|([ \t\n\r]+)");
    
    std::cout << "组合NFA: " << std::format("{}", combined_nfa) << std::endl;
    
    // 将NFA转换为DFA进行测试
    regex::dfa combined_dfa = regex::to_dfa(combined_nfa);
    
    // 测试各种输入并添加断言
    std::cout << "\n--- 测试组合DFA ---" << std::endl;
    bool result1 = combined_dfa.match("if");
    std::cout << "匹配 'if': " << result1 << std::endl;
    assert(result1 == true);
    
    bool result2 = combined_dfa.match("variable");
    std::cout << "匹配 'variable': " << result2 << std::endl;
    assert(result2 == true);
    
    bool result3 = combined_dfa.match("123");
    std::cout << "匹配 '123': " << result3 << std::endl;
    assert(result3 == true);
    
    bool result4 = combined_dfa.match(" \t");
    std::cout << "匹配 '  \t': " << result4 << std::endl;
    assert(result4 == true);
    
    bool result5 = combined_dfa.match("!@#");  // 特殊字符，不符合任何模式
    std::cout << "匹配 '!@#': " << result5 << std::endl;
    assert(result5 == false);  // 特殊字符不符合任何模式，所以不应该匹配
    
    // 使用新的查找功能来测试子串匹配
    std::cout << "\n--- 测试查找功能 ---" << std::endl;
    int find_result1 = combined_dfa.find_match("if (x > 0)");
    std::cout << "在 'if (x > 0)' 中查找: " << find_result1 << std::endl;
    assert(find_result1 >= 0); // 应该找到匹配
    
    int find_result2 = combined_dfa.find_match("int var = 123");
    std::cout << "在 'int var = 123' 中查找: " << find_result2 << std::endl;
    assert(find_result2 >= 0); // 应该找到匹配

    // 更精确的测试：验证是否可以区分不同的规则
    std::cout << "\n--- 验证规则区分能力 ---" << std::endl;
    
    // 创建单独的DFA来区分不同类型的token
    regex::dfa keyword_dfa = regex::build_dfa("if|else|while|for");
    regex::dfa identifier_dfa = regex::build_dfa("[a-zA-Z_][a-zA-Z0-9_]*");
    regex::dfa number_dfa = regex::build_dfa("[0-9]+");
    
    bool kw_result1 = keyword_dfa.match("if");
    bool id_result1 = identifier_dfa.match("if");
    bool num_result1 = number_dfa.match("if");
    std::cout << "'if' 是关键字: " << kw_result1 << std::endl;
    std::cout << "'if' 是标识符: " << id_result1 << std::endl;
    std::cout << "'if' 是数字: " << num_result1 << std::endl;
    assert(kw_result1 == true);
    assert(id_result1 == true);  // 'if' 也符合标识符模式，所以会被匹配
    assert(num_result1 == false);
    
    bool kw_result2 = keyword_dfa.match("variable");
    bool id_result2 = identifier_dfa.match("variable");
    bool num_result2 = number_dfa.match("variable");
    std::cout << "\n'variable' 是关键字: " << kw_result2 << std::endl;
    std::cout << "'variable' 是标识符: " << id_result2 << std::endl;
    std::cout << "'variable' 是数字: " << num_result2 << std::endl;
    assert(kw_result2 == false);
    assert(id_result2 == true);
    assert(num_result2 == false);
    
    bool kw_result3 = keyword_dfa.match("456");
    bool id_result3 = identifier_dfa.match("456");
    bool num_result3 = number_dfa.match("456");
    std::cout << "\n'456' 是关键字: " << kw_result3 << std::endl;
    std::cout << "'456' 是标识符: " << id_result3 << std::endl;
    std::cout << "'456' 是数字: " << num_result3 << std::endl;
    assert(kw_result3 == false);
    assert(id_result3 == false);
    assert(num_result3 == true);
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
    bool result1 = combined_dfa.match("abc");
    std::cout << "匹配 'abc': " << result1 << std::endl;
    assert(result1 == true);
    
    bool result2 = combined_dfa.match("xyz");
    std::cout << "匹配 'xyz': " << result2 << std::endl;
    assert(result2 == true);
    
    bool result3 = combined_dfa.match("abcx");
    std::cout << "匹配 'abcx': " << result3 << std::endl;
    assert(result3 == false);
    
    bool result4 = combined_dfa.match("123");
    std::cout << "匹配 '123': " << result4 << std::endl;
    assert(result4 == false);
}

int main()
{
    test_parallel_nfa();
    test_manual_merge();
     
    std::cout << "\n=== 所有多规则并行匹配测试通过 ===" << std::endl;
    std::cout << "结论：该正则引擎支持类似Flex的多规则并行匹配功能，" << std::endl;
    std::cout << "通过选择操作(|)可以将多个规则合并成一个NFA进行并行匹配。" << std::endl;
     
    return 0;
}
