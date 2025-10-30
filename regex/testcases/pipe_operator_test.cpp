#include "regex.hpp"
#include <iostream>
#include <cassert>

int main()
{
    std::cout << "开始管道运算符测试..." << std::endl;

    try {
        // 测试1: 使用NFA通过管道运算符转换为DFA
        {
            std::cout << "\n测试1: NFA | to_dfa" << std::endl;
            regex::nfa nfa = regex::build_nfa("a*b");
            regex::dfa dfa = nfa | regex::to_dfa;
            
            // 测试DFA的匹配结果
            assert(dfa.match("b") == true);
            assert(dfa.match("ab") == true);
            assert(dfa.match("aab") == true);
            assert(dfa.match("aaab") == true);
            assert(dfa.match("a") == false);
            assert(dfa.match("aa") == false);
            assert(dfa.match("ba") == false);
            assert(dfa.match("") == false);
            
            std::cout << "测试1通过!" << std::endl;
        }

        // 测试2: 使用DFA通过管道运算符进行最小化
        {
            std::cout << "\n测试2: DFA | minimize" << std::endl;
            regex::dfa dfa = regex::build_dfa("(a|b)*a(a|b)");
            regex::dfa minimized_dfa = dfa | regex::minimize;
            
            // 比较原始DFA和最小化DFA的匹配结果，应该相同
            assert(dfa.match("aa") == minimized_dfa.match("aa"));
            assert(dfa.match("ab") == minimized_dfa.match("ab"));
            assert(dfa.match("ba") == minimized_dfa.match("ba"));
            assert(dfa.match("bb") == minimized_dfa.match("bb"));
            assert(dfa.match("aaa") == minimized_dfa.match("aaa"));
            assert(dfa.match("aba") == minimized_dfa.match("aba"));
            assert(dfa.match("aab") == minimized_dfa.match("aab"));
            assert(dfa.match("xyz") == minimized_dfa.match("xyz"));
            
            std::cout << "测试2通过!" << std::endl;
        }

        // 测试3: 链式管道操作 NFA | to_dfa | minimize
        {
            std::cout << "\n测试3: NFA | to_dfa | minimize" << std::endl;
            regex::nfa nfa = regex::build_nfa("a+");
            regex::dfa dfa = nfa | regex::to_dfa;
            regex::dfa minimized_dfa = dfa | regex::minimize;
            
            // 比较转换后的DFA和最小化DFA的匹配结果，应该相同
            assert(dfa.match("a") == minimized_dfa.match("a"));
            assert(dfa.match("aa") == minimized_dfa.match("aa"));
            assert(dfa.match("aaa") == minimized_dfa.match("aaa"));
            assert(dfa.match("") == minimized_dfa.match(""));
            assert(dfa.match("b") == minimized_dfa.match("b"));
            assert(dfa.match("ab") == minimized_dfa.match("ab"));
            
            std::cout << "测试3通过!" << std::endl;
        }

        // 测试4: 直接使用字符串构建DFA并最小化
        {
            std::cout << "\n测试4: 字符串直接构建DFA并最小化" << std::endl;
            regex::dfa dfa = regex::build_dfa("(a|b)*");
            regex::dfa minimized_dfa = dfa | regex::minimize;
            
            // 比较原始DFA和最小化DFA的匹配结果，应该相同
            assert(dfa.match("") == minimized_dfa.match(""));
            assert(dfa.match("a") == minimized_dfa.match("a"));
            assert(dfa.match("b") == minimized_dfa.match("b"));
            assert(dfa.match("ab") == minimized_dfa.match("ab"));
            assert(dfa.match("ba") == minimized_dfa.match("ba"));
            assert(dfa.match("aba") == minimized_dfa.match("aba"));
            assert(dfa.match("xyz") == minimized_dfa.match("xyz")); // 包含非a,b字符，应该都为false
            
            std::cout << "测试4通过!" << std::endl;
        }

        // 测试5: 复杂正则表达式的管道操作
        {
            std::cout << "\n测试5: 复杂正则表达式 (a|b)*abb" << std::endl;
            regex::nfa nfa = regex::build_nfa("(a|b)*abb");
            regex::dfa dfa = nfa | regex::to_dfa;
            regex::dfa minimized_dfa = dfa | regex::minimize;
            
            // 比较原始DFA和最小化DFA的匹配结果，应该相同
            assert(dfa.match("abb") == minimized_dfa.match("abb"));
            assert(dfa.match("aabb") == minimized_dfa.match("aabb"));
            assert(dfa.match("ababb") == minimized_dfa.match("ababb"));
            assert(dfa.match("abbabb") == minimized_dfa.match("abbabb"));
            assert(dfa.match("ab") == minimized_dfa.match("ab"));
            assert(dfa.match("abbab") == minimized_dfa.match("abbab"));
            assert(dfa.match("xyz") == minimized_dfa.match("xyz"));
            assert(dfa.match("abab") == minimized_dfa.match("abab"));
            
            std::cout << "测试5通过!" << std::endl;
        }

        // 测试6: 更复杂的正则表达式，测试NFA和DFA状态管理
        {
            std::cout << "\n测试6: 复杂正则表达式 (a|b)+c(d|e)*f" << std::endl;
            regex::nfa nfa = regex::build_nfa("(a|b)+c(d|e)*f");
            regex::dfa dfa = nfa | regex::to_dfa;
            regex::dfa minimized_dfa = nfa | regex::to_dfa | regex::minimize;
            
            // 比较原始DFA和最小化DFA的匹配结果，应该相同
            assert(dfa.match("acdf") == minimized_dfa.match("acdf"));
            assert(dfa.match("bcf") == minimized_dfa.match("bcf"));
            assert(dfa.match("abacdef") == minimized_dfa.match("abacdef"));
            assert(dfa.match("bbaacddf") == minimized_dfa.match("bbaacddf"));
            assert(dfa.match("abacdeef") == minimized_dfa.match("abacdeef"));
            assert(dfa.match("ab") == minimized_dfa.match("ab")); // 应该不匹配
            assert(dfa.match("ac") == minimized_dfa.match("ac")); // 应该不匹配
            assert(dfa.match("af") == minimized_dfa.match("af")); // 应该不匹配
            assert(dfa.match("xyz") == minimized_dfa.match("xyz"));
            assert(dfa.match("acde") == minimized_dfa.match("acde")); // 应该不匹配，缺少f
            assert(dfa.match("cdef") == minimized_dfa.match("cdef")); // 应该不匹配，缺少a或b开头
            assert(dfa.match("abcdef") == minimized_dfa.match("abcdef"));
            
            std::cout << "测试6通过!" << std::endl;
        }

        std::cout << "\n所有管道运算符测试通过!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
