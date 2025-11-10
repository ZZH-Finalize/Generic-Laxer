#include <iostream>
#include <format>
#include <string_view>
#include <cassert>
#include "regex.hpp"

int main(const int argc, const char** argv)
{
    std::cout << "测试NFA构造..." << std::flush;

    try {
        // 测试NFA构造
        std::cout << "尝试构建NFA..." << std::flush;
        regex::nfa nfa_obj = regex::build_nfa("a");
        std::cout << "NFA构造成功: " << std::format("{}", nfa_obj) << std::endl;

        // 验证NFA基本信息
        assert(nfa_obj.get_states().size() > 0);
        std::cout << "NFA状态数量: " << nfa_obj.get_states().size() << std::endl;
        
        assert(nfa_obj.get_start() >= 0);
        std::cout << "起始状态ID: " << nfa_obj.get_start() << std::endl;
        
        // assert(nfa_obj.get_final() >= 0);
        // std::cout << "最终状态ID: " << nfa_obj.get_final() << std::endl;

        // 将NFA转换为DFA并测试匹配
        regex::dfa dfa_obj = regex::to_dfa(nfa_obj);
        
        // 测试匹配结果
        auto match_a = dfa_obj.match("a");
        std::cout << "NFA转换后的DFA匹配 'a': " << match_a.has_value() << std::endl;
        assert(match_a.has_value());
        
        auto match_b = dfa_obj.match("b");
        std::cout << "NFA转换后的DFA匹配 'b': " << match_b.has_value() << std::endl;
        assert(!match_b.has_value());
        
        auto match_empty = dfa_obj.match("");
        std::cout << "NFA转换后的DFA匹配 '': " << match_empty.has_value() << std::endl;
        assert(!match_empty.has_value());

        std::cout << "NFA测试通过!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "异常: " << e.what() << std::endl;
        return 1; // 测试失败返回非零值
    }

    return 0;
}
