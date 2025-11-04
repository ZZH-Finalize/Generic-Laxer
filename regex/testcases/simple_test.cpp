#include <iostream>
#include <format>
#include <string_view>
#include <cassert>
#include "regex/regex.hpp"

int main(const int argc, const char** argv)
{
    std::cout << "测试NFA构造..." << std::endl;
    
    try {
        // 先测试NFA构造
        regex::nfa nfa_obj = regex::build_nfa("a");
        std::cout << "NFA构造成功: " << std::format("{}", nfa_obj) << std::endl;
        
        std::cout << "测试DFA构造..." << std::endl;
        // 测试DFA构造
        regex::dfa dfa_obj = regex::to_dfa(nfa_obj);
        std::cout << "DFA构造成功" << std::endl;
        
        // 测试匹配并添加断言来验证结果
        bool match_a = dfa_obj.match("a");
        std::cout << "测试匹配 'a': " << match_a << std::endl;
        assert(match_a == true);
        
        bool match_b = dfa_obj.match("b");
        std::cout << "测试匹配 'b': " << match_b << std::endl;
        assert(match_b == false);
        
        // 添加更多测试用例
        bool match_aa = dfa_obj.match("aa");
        std::cout << "测试匹配 'aa': " << match_aa << std::endl;
        assert(match_aa == false);
        
        bool match_empty = dfa_obj.match("");
        std::cout << "测试匹配 '': " << match_empty << std::endl;
        assert(match_empty == false);

        std::cout << "简单测试通过!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "异常: " << e.what() << std::endl;
        return 1; // 测试失败返回非零值
    }

    return 0;
}
