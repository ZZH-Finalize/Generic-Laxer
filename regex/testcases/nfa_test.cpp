#include <iostream>
#include <format>
#include <string_view>
#include "regex/regex.hpp"

int main(const int argc, const char** argv)
{
    std::cout << "测试NFA构造..." << std::flush;

    try {
        // 测试NFA构造
        std::cout << "尝试构建NFA..." << std::flush;
        regex::nfa nfa_obj = regex::build_nfa("a");
        std::cout << "NFA构造成功: " << nfa_obj.to_string() << std::endl;

        // 打印详细信息
        std::cout << "NFA状态数量: " << nfa_obj.get_states().size() << std::endl;
        std::cout << "起始状态ID: " << nfa_obj.get_start() << std::endl;
        std::cout << "最终状态ID: " << nfa_obj.get_final() << std::endl;

    } catch (const std::exception& e) {
        std::cout << "异常: " << e.what() << std::endl;
    }

    return 0;
}
