#include "nfa.hpp"
#include "regex/regex.hpp"
#include <cassert>
#include <iostream>

int main()
{
    std::cout << "Starting updated NFA test..." << std::endl;

    // 创建Laxer::nfa对象
    laxer::nfa laxer_nfa;

    std::cout << "Create Laxer::nfa object completed" << std::endl;

    // 构建几个测试NFA
    regex::nfa keyword_nfa    = regex::build_nfa("if");
    regex::nfa identifier_nfa = regex::build_nfa("variable");
    regex::nfa number_nfa     = regex::build_nfa("123");

    std::cout << "Build test NFA completed" << std::endl;

    // 添加第一个NFA
    laxer_nfa.add_nfa(keyword_nfa);
    std::cout << "Add keyword NFA completed" << std::endl;

    // 添加第二个NFA
    laxer_nfa.add_nfa(identifier_nfa);
    std::cout << "Add identifier NFA completed" << std::endl;

    // 添加第三个NFA
    laxer_nfa.add_nfa(number_nfa);
    std::cout << "Add number NFA completed" << std::endl;

    auto totol = keyword_nfa.get_states().size() + identifier_nfa.get_states().size()
                 + number_nfa.get_states().size();

    // 验证状态数量
    std::cout << "Three nfa state count: " << totol << std::endl;
    std::cout << "Total state count: " << laxer_nfa.get_states().size() << std::endl;
    std::cout << "Accept state count: " << laxer_nfa.get_accept_states().size()
              << std::endl;
    std::cout << "Start state ID: " << laxer_nfa.get_start() << std::endl;

    assert(laxer_nfa.get_states().size() == totol + 2);
    assert(laxer_nfa.get_accept_states().size() == 3);

    return 0;
}
