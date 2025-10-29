#include "regex.hpp"
#include <iostream>
#include <cassert>

int main()
{
    std::cout << "开始高级DFA最小化测试..." << std::endl;

    try {
        // 测试1: 简单的正则表达式
        {
            std::cout << "\n测试1: (a|b)*a(a|b)" << std::endl;
            regex::dfa dfa         = regex::build_dfa("(a|b)*a(a|b)");
            size_t original_states = dfa.get_state_count();
            std::cout << "原始DFA状态数: " << original_states << std::endl;

            // 测试原始DFA
            assert(dfa.match("aa") == true);
            assert(dfa.match("ab") == true);
            assert(dfa.match("ba") == false);
            assert(dfa.match("bb") == false);
            assert(dfa.match("aaa") == true);
            assert(dfa.match("aba") == false);
            assert(dfa.match("aab") == true);

            // 最小化DFA
            dfa                     = regex::minimize(dfa);
            size_t minimized_states = dfa.get_state_count();
            std::cout << "最小化后DFA状态数: " << minimized_states << std::endl;

            // 测试最小化后的DFA，结果应该相同
            assert(dfa.match("aa") == true);
            assert(dfa.match("ab") == true);
            assert(dfa.match("ba") == false);
            assert(dfa.match("bb") == false);
            assert(dfa.match("aaa") == true);
            assert(dfa.match("aba") == false);
            assert(dfa.match("aab") == true);

            std::cout << "测试1通过!" << std::endl;
        }

        // 测试2: 简单的a+模式
        {
            std::cout << "\n测试2: a+" << std::endl;
            regex::dfa dfa         = regex::build_dfa("a+");
            size_t original_states = dfa.get_state_count();
            std::cout << "原始DFA状态数: " << original_states << std::endl;

            // 测试原始DFA
            assert(dfa.match("a") == true);
            assert(dfa.match("aa") == true);
            assert(dfa.match("aaa") == true);
            assert(dfa.match("") == false);
            assert(dfa.match("b") == false);

            // 最小化DFA
            dfa                     = regex::minimize(dfa);
            size_t minimized_states = dfa.get_state_count();
            std::cout << "最小化后DFA状态数: " << minimized_states << std::endl;

            // 测试最小化后的DFA，结果应该相同
            assert(dfa.match("a") == true);
            assert(dfa.match("aa") == true);
            assert(dfa.match("aaa") == true);
            assert(dfa.match("") == false);
            assert(dfa.match("b") == false);

            std::cout << "测试2通过!" << std::endl;
        }

        // 测试3: (a|b)* 模式（匹配所有a和b组成的字符串）
        {
            std::cout << "\n测试3: (a|b)*" << std::endl;
            regex::dfa dfa         = regex::build_dfa("(a|b)*");
            size_t original_states = dfa.get_state_count();
            std::cout << "原始DFA状态数: " << original_states << std::endl;

            // 测试原始DFA
            assert(dfa.match("") == true);
            assert(dfa.match("a") == true);
            assert(dfa.match("b") == true);
            assert(dfa.match("ab") == true);
            assert(dfa.match("ba") == true);
            assert(dfa.match("aba") == true);
            assert(dfa.match("xyz") == false); // 包含非a,b字符

            // 最小化DFA
            dfa                     = regex::minimize(dfa);
            size_t minimized_states = dfa.get_state_count();
            std::cout << "最小化后DFA状态数: " << minimized_states << std::endl;

            // 测试最小化后的DFA，结果应该相同
            assert(dfa.match("") == true);
            assert(dfa.match("a") == true);
            assert(dfa.match("b") == true);
            assert(dfa.match("ab") == true);
            assert(dfa.match("ba") == true);
            assert(dfa.match("aba") == true);
            assert(dfa.match("xyz") == false); // 包含非a,b字符

            std::cout << "测试3通过!" << std::endl;
        }

        std::cout << "\n所有DFA最小化测试通过!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
