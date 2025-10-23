#pragma once

#include <format>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <memory>
#include <algorithm>
#include <functional>
#include <stack>
#include <queue>

namespace Regex {

    // 有限状态自动机的节点
    struct State
    {
        int id;
        bool is_final;
        std::map<char, std::vector<int>> transitions; // 字符 -> 目标状态列表
        static int global_id_counter;

        State(): id(global_id_counter++), is_final(false)
        {
            std::cout << std::format("new state with id: {}", this->id) << std::endl;
        }

        void add_transition(char input, int to_state)
        {
            transitions[input].push_back(to_state);
        }

        void add_epsilon_transition(int to_state)
        {
            transitions['\0'].push_back(to_state); // 使用空字符表示epsilon转换
        }
    };

    // NFA (非确定性有限自动机)
    class NFA {
       public:
        std::vector<std::shared_ptr<State>> states;
        int start_state;
        int final_state;

        NFA(): start_state(-1), final_state(-1)
        {
            std::cout << "new nfa" << std::endl;
        }

        int add_state()
        {
            auto newState = std::make_shared<State>();
            states.push_back(newState);
            return states.size() - 1;
        }

        void set_start(int state)
        {
            start_state = state;
        }

        void set_final(int state)
        {
            final_state = state;
        }
    };

    // 正则表达式解析器和转换器
    class RegexToNFA {
       private:
        std::string pattern;
        int pos;

        // 解析单个字符或字符类
        NFA parse_atom()
        {
            NFA nfa;
            if (pos >= pattern.length()) {
                return nfa;
            }

            char c = pattern[pos++];

            switch (c) {
                case '(': nfa = parse_group(); break;
                case '.': nfa = create_wildcard_nfa(); break;
                case '[': nfa = parse_char_class(); break;
                case '\\':
                    if (pos < pattern.length()) {
                        c   = pattern[pos++];
                        nfa = create_char_nfa(c);
                    } else {
                        nfa = create_char_nfa('\\');
                    }
                    break;
                default: nfa = create_char_nfa(c); break;
            }

            return nfa;
        }

        // 解析括号组
        NFA parse_group()
        {
            NFA inner_nfa = parse_expression();

            if (pos < pattern.length() && pattern[pos] == ')') {
                pos++;
            }

            return apply_quantifier(inner_nfa);
        }

        // 解析字符类 [abc] 或 [a-z]
        NFA parse_char_class()
        {
            NFA nfa;
            int start_state = nfa.add_state();
            int end_state   = nfa.add_state();

            nfa.set_start(start_state);
            nfa.set_final(end_state);

            if (pos < pattern.length() && pattern[pos] == '^') {
                pos++; // 处理否定字符类 [^abc]
                // 简化处理，这里只处理基本字符类
            }

            std::set<char> chars;
            while (pos < pattern.length() && pattern[pos] != ']') {
                char c = pattern[pos++];
                if (pos < pattern.length() && pattern[pos] == '-'
                    && pos + 1 < pattern.length() && pattern[pos + 1] != ']') {
                    pos++; // 跳过 '-'
                    char end_c = pattern[pos++];
                    // 添加范围内的所有字符
                    for (char range_c = c; range_c <= end_c; range_c++) {
                        chars.insert(range_c);
                    }
                } else {
                    chars.insert(c);
                }
            }

            if (pos < pattern.length() && pattern[pos] == ']') {
                pos++;
            }

            // 为每个字符添加转换
            for (char c : chars) {
                nfa.states[start_state]->add_transition(c, end_state);
            }

            return apply_quantifier(nfa);
        }

        // 创建匹配单个字符的NFA
        NFA create_char_nfa(char c)
        {
            NFA nfa;
            int start = nfa.add_state();
            int end   = nfa.add_state();

            nfa.set_start(start);
            nfa.set_final(end);
            nfa.states[start]->add_transition(c, end);

            return apply_quantifier(nfa);
        }

        // 创建匹配任意字符的NFA
        NFA create_wildcard_nfa()
        {
            NFA nfa;
            int start = nfa.add_state();
            int end   = nfa.add_state();

            nfa.set_start(start);
            nfa.set_final(end);

            // 匹配任意字符（简化处理，实际应用中需要定义字符集）
            for (int c = 32; c <= 126; c++) { // 常见可打印字符
                nfa.states[start]->add_transition((char) c, end);
            }

            return apply_quantifier(nfa);
        }

        // 应用量词 (?, *, +)
        NFA apply_quantifier(NFA& nfa)
        {
            if (pos >= pattern.length()) {
                return nfa;
            }

            char quantifier = pattern[pos];

            // 记录原始nfa的start/final状态值
            int original_start = nfa.start_state;
            int original_final = nfa.final_state;

            switch (quantifier) {
                case '?': // 0或1次
                    pos++;
                    {
                        NFA result;
                        int new_start = result.add_state();
                        int new_end   = result.add_state();

                        // 临时保存result的start/final状态，防止被copy_nfa覆盖
                        int old_start = result.start_state;
                        int old_final = result.final_state;

                        // 复制原NFA
                        copy_nfa(nfa, result);

                        // 恢复result的start/final状态
                        result.set_start(new_start);
                        result.set_final(new_end);

                        // 添加epsilon转换绕过原NFA（允许0次匹配）
                        result.states[new_start]->add_epsilon_transition(
                            original_start + 2); // 原NFA开始
                        result.states[new_start]->add_epsilon_transition(
                            new_end); // 直接到结束
                        result.states[original_final + 2]->add_epsilon_transition(
                            new_end); // 原NFA结束到新结束

                        return result;
                    }

                case '*': // 0次或多次
                    pos++;
                    {
                        NFA result;
                        int new_start = result.add_state();
                        int new_end   = result.add_state();

                        // 临时保存result的start/final状态，防止被copy_nfa覆盖
                        int old_start = result.start_state;
                        int old_final = result.final_state;

                        // 复制原NFA
                        copy_nfa(nfa, result);

                        // 恢复result的start/final状态
                        result.set_start(new_start);
                        result.set_final(new_end);

                        // 添加epsilon转换形成循环
                        result.states[new_start]->add_epsilon_transition(
                            original_start + 2); // 原NFA开始
                        result.states[new_start]->add_epsilon_transition(
                            new_end); // 直接到结束（0次匹配）
                        result.states[original_final + 2]->add_epsilon_transition(
                            original_start + 2); // 循环回到开始
                        result.states[original_final + 2]->add_epsilon_transition(
                            new_end); // 结束匹配

                        return result;
                    }

                case '+': // 1次或多次
                    pos++;
                    {
                        NFA result;
                        int new_start = result.add_state();
                        int new_end   = result.add_state();

                        // 临时保存result的start/final状态，防止被copy_nfa覆盖
                        int old_start = result.start_state;
                        int old_final = result.final_state;

                        // 复制原NFA
                        copy_nfa(nfa, result);

                        // 恢复result的start/final状态
                        result.set_start(new_start);
                        result.set_final(new_end);

                        // 添加epsilon转换形成循环，但必须匹配至少一次
                        result.states[new_start]->add_epsilon_transition(original_start
                                                                         + 2); // 开始匹配
                        result.states[original_final + 2]->add_epsilon_transition(
                            original_start + 2); // 循环回到开始
                        result.states[original_final + 2]->add_epsilon_transition(
                            new_end); // 结束匹配

                        return result;
                    }

                default: return nfa;
            }
        }

        // 复制NFA到另一个NFA
        void copy_nfa(NFA& source, NFA& target)
        {
            // 计算目标NFA中状态的偏移量
            int offset = target.states.size();

            // 复制所有状态
            for (auto& state : source.states) {
                auto newState      = std::make_shared<State>();
                newState->is_final = state->is_final;
                newState->id       = State::global_id_counter++;

                // 复制转换
                for (auto& transition : state->transitions) {
                    for (int dest_state : transition.second) {
                        newState->add_transition(transition.first, dest_state + offset);
                    }
                }

                target.states.push_back(newState);
            }

            // 更新起始和结束状态索引（只在目标NFA还没有设置起始状态时才更新）
            if (target.start_state == -1) {
                target.start_state = source.start_state + offset;
            }
            target.final_state = source.final_state + offset;
        }

        // 解析序列 (concatenation)
        NFA parse_sequence()
        {
            NFA result = parse_atom();

            while (pos < pattern.length() && pattern[pos] != ')' && pattern[pos] != '|'
                   && pattern[pos] != '\0') {
                NFA next = parse_atom();

                // 连接两个NFA
                NFA combined;
                copy_nfa(result, combined);

                int offset = combined.states.size();
                copy_nfa(next, combined);

                // 连接第一个NFA的终态和第二个NFA的初态（通过epsilon转换）
                combined.states[result.final_state]->add_epsilon_transition(
                    next.start_state + offset);

                combined.set_start(result.start_state);
                combined.set_final(next.final_state + offset);

                result = combined;
            }

            return result;
        }

        // 解析选择 (alternation - |)
        NFA parse_expression()
        {
            NFA left = parse_sequence();

            if (pos < pattern.length() && pattern[pos] == '|') {
                pos++;                          // 跳过 '|'
                NFA right = parse_expression(); // 递归解析右侧

                NFA result;
                int new_start = result.add_state();
                int new_end   = result.add_state();

                // 记录原始left和right的start/final状态值
                int left_start  = left.start_state;
                int left_final  = left.final_state;
                int right_start = right.start_state;
                int right_final = right.final_state;

                // 临时保存result的start/final状态，防止被copy_nfa覆盖
                int old_start = result.start_state;
                int old_final = result.final_state;

                // 复制左侧NFA
                copy_nfa(left, result);

                int offset = result.states.size();
                // 复制右侧NFA
                copy_nfa(right, result);

                // 恢复result的start/final状态
                result.set_start(new_start);
                result.set_final(new_end);

                // 添加epsilon转换从新开始状态到两个分支
                result.states[new_start]->add_epsilon_transition(left_start
                                                                 + 2); // left分支
                result.states[new_start]->add_epsilon_transition(right_start
                                                                 + offset); // right分支

                // 添加epsilon转换从两个分支的终态到新终态
                result.states[left_final + 2]->add_epsilon_transition(
                    new_end); // left终态
                result.states[right_final + offset]->add_epsilon_transition(
                    new_end); // right终态

                return result;
            }

            return left;
        }

       public:
        // 将正则表达式转换为NFA
        NFA convert(const std::string& regex_pattern)
        {
            pattern                  = regex_pattern;
            pos                      = 0;
            State::global_id_counter = 0; // 重置ID计数器

            NFA nfa = parse_expression();

            // 确保终态是终态
            if (nfa.final_state != -1) {
                nfa.states[nfa.final_state]->is_final = true;
            }

            return nfa;
        }

        // 打印NFA结构（用于调试）
        void print_nfa(const NFA& nfa)
        {
            std::cout << "NFA Structure:\n";
            std::cout << "Start State: " << nfa.start_state << "\n";
            std::cout << "Final State: " << nfa.final_state << "\n";

            for (size_t i = 0; i < nfa.states.size(); i++) {
                std::cout << "State " << i << (nfa.states[i]->is_final ? " (final)" : "")
                          << ":\n";

                for (const auto& transition : nfa.states[i]->transitions) {
                    std::cout << "  ";
                    if (transition.first == '\0') {
                        std::cout << "ε -> ";
                    } else {
                        std::cout << transition.first << " -> ";
                    }

                    for (int dest : transition.second) {
                        std::cout << dest << " ";
                    }
                    std::cout << "\n";
                }
            }
        }
    };

    // NFA状态ID计数器的定义
    int State::global_id_counter = 0;

    // 简化的NFA匹配器
    class NFA_Matcher {
       public:
        // 使用子集构造算法检查字符串是否匹配NFA
        static bool match(const NFA& nfa, const std::string& input)
        {
            if (nfa.start_state == -1 || nfa.final_state == -1) {
                return input.empty();
            }

            // 获取起始状态的epsilon闭包
            std::set<int> current_states = epsilon_closure(nfa, {nfa.start_state});

            for (char c : input) {
                std::set<int> next_states;

                // 对于当前状态集中的每个状态
                for (int state_id : current_states) {
                    if (state_id < static_cast<int>(nfa.states.size())) {
                        auto it = nfa.states[state_id]->transitions.find(c);
                        if (it != nfa.states[state_id]->transitions.end()) {
                            // 添加所有通过字符c可到达的状态
                            for (int next_state : it->second) {
                                std::set<int> closure =
                                    epsilon_closure(nfa, {next_state});
                                next_states.insert(closure.begin(), closure.end());
                            }
                        }
                    }
                }

                current_states = next_states;

                // 如果没有可到达的状态，则匹配失败
                if (current_states.empty()) {
                    return false;
                }
            }

            // 检查最终状态集中是否包含终态
            for (int state_id : current_states) {
                if (state_id < static_cast<int>(nfa.states.size())
                    && nfa.states[state_id]->is_final) {
                    return true;
                }
            }

            return false;
        }

       private:
        // 计算epsilon闭包
        static std::set<int> epsilon_closure(const NFA& nfa, const std::set<int>& states)
        {
            std::set<int> closure = states;
            std::queue<int> queue;

            // 将所有初始状态加入队列
            for (int state : states) {
                queue.push(state);
            }

            // BFS查找所有通过epsilon转换可达的状态
            while (!queue.empty()) {
                int current = queue.front();
                queue.pop();

                // 检查是否有epsilon转换
                auto it = nfa.states[current]->transitions.find(
                    '\0'); // epsilon转换用空字符表示
                if (it != nfa.states[current]->transitions.end()) {
                    for (int next_state : it->second) {
                        if (closure.insert(next_state).second) {
                            queue.push(next_state);
                        }
                    }
                }
            }

            return closure;
        }
    };

    // DFA (确定性有限自动机) - 用于更高效的匹配
    struct DFAState
    {
        int id;
        bool is_final;
        std::map<char, int> transitions; // 字符 -> 目标状态
        static int global_id_counter;

        DFAState(): id(global_id_counter++), is_final(false)
        {
        }

        void add_transition(char input, int to_state)
        {
            transitions[input] = to_state;
        }
    };

    class DFA {
       public:
        std::vector<std::shared_ptr<DFAState>> states;
        int start_state;
        int final_state;

        DFA(): start_state(-1), final_state(-1)
        {
        }

        int add_state()
        {
            auto newState = std::make_shared<DFAState>();
            states.push_back(newState);
            return states.size() - 1;
        }

        void set_start(int state)
        {
            start_state = state;
        }

        void set_final(int state)
        {
            final_state = state;
        }
    };

    // NFA到DFA的转换器（子集构造算法）
    class NFAtoDFAConverter {
       public:
        static DFA convert(const NFA& nfa)
        {
            DFA dfa;
            if (nfa.start_state == -1 || nfa.final_state == -1) {
                return dfa;
            }

            // 用于映射NFA状态集合到DFA状态的映射表
            std::map<std::set<int>, int> subset_to_dfa_state;
            std::vector<std::set<int>> dfa_states_list; // 每个DFA状态对应的NFA状态集合

            // 获取起始状态的epsilon闭包作为DFA的起始状态
            std::set<int> start_closure = epsilon_closure(nfa, {nfa.start_state});
            int dfa_start_state         = dfa.add_state();

            // 检查起始状态集合是否包含NFA终态
            dfa.states[dfa_start_state]->is_final =
                contains_final_state(nfa, start_closure, nfa.final_state);

            subset_to_dfa_state[start_closure] = dfa_start_state;
            dfa_states_list.push_back(start_closure);
            dfa.set_start(dfa_start_state);

            // 使用队列进行广度优先搜索
            std::queue<std::set<int>> work_queue;
            work_queue.push(start_closure);

            while (!work_queue.empty()) {
                std::set<int> current_subset = work_queue.front();
                work_queue.pop();

                // 获取当前DFA状态在列表中的索引
                int current_dfa_state_idx = -1;
                for (size_t i = 0; i < dfa_states_list.size(); i++) {
                    if (dfa_states_list[i] == current_subset) {
                        current_dfa_state_idx = i;
                        break;
                    }
                }

                if (current_dfa_state_idx == -1) continue;

                // 获取所有可能的输入字符
                std::set<char> input_chars = get_input_chars(nfa, current_subset);

                for (char input_char : input_chars) {
                    // 计算从当前状态子集通过input_char可以到达的NFA状态集合
                    std::set<int> next_nfa_states = move(nfa, current_subset, input_char);
                    std::set<int> next_closure    = epsilon_closure(nfa, next_nfa_states);

                    if (next_closure.empty()) continue;

                    // 检查是否已存在对应的DFA状态
                    if (subset_to_dfa_state.find(next_closure)
                        == subset_to_dfa_state.end()) {
                        // 创建新的DFA状态
                        int new_dfa_state = dfa.add_state();
                        dfa.states[new_dfa_state]->is_final =
                            contains_final_state(nfa, next_closure, nfa.final_state);

                        subset_to_dfa_state[next_closure] = new_dfa_state;
                        dfa_states_list.push_back(next_closure);
                        work_queue.push(next_closure);
                    }

                    // 添加从当前DFA状态到新DFA状态的转换
                    int target_dfa_state = subset_to_dfa_state[next_closure];
                    dfa.states[current_dfa_state_idx]->add_transition(input_char,
                                                                      target_dfa_state);
                }
            }

            // 设置DFA的终态（可以是任意包含NFA终态的状态）
            for (size_t i = 0; i < dfa_states_list.size(); i++) {
                if (dfa.states[i]->is_final) {
                    dfa.set_final(i);
                    break;
                }
            }

            return dfa;
        }

       private:
        // 计算epsilon闭包
        static std::set<int> epsilon_closure(const NFA& nfa, const std::set<int>& states)
        {
            std::set<int> closure = states;
            std::queue<int> queue;

            // 将所有初始状态加入队列
            for (int state : states) {
                queue.push(state);
            }

            // BFS查找所有通过epsilon转换可达的状态
            while (!queue.empty()) {
                int current = queue.front();
                queue.pop();

                // 检查是否有epsilon转换
                auto it = nfa.states[current]->transitions.find(
                    '\0'); // epsilon转换用空字符表示
                if (it != nfa.states[current]->transitions.end()) {
                    for (int next_state : it->second) {
                        if (closure.insert(next_state).second) {
                            queue.push(next_state);
                        }
                    }
                }
            }

            return closure;
        }

        // 从状态集合通过给定输入字符可以到达的状态集合
        static std::set<int> move(const NFA& nfa, const std::set<int>& from_states,
                                  char input)
        {
            std::set<int> result;

            for (int state_id : from_states) {
                if (state_id < static_cast<int>(nfa.states.size())) {
                    auto it = nfa.states[state_id]->transitions.find(input);
                    if (it != nfa.states[state_id]->transitions.end()) {
                        for (int next_state : it->second) {
                            result.insert(next_state);
                        }
                    }
                }
            }

            return result;
        }

        // 获取状态集合中所有可能的输入字符
        static std::set<char> get_input_chars(const NFA& nfa, const std::set<int>& states)
        {
            std::set<char> input_chars;

            for (int state_id : states) {
                if (state_id < static_cast<int>(nfa.states.size())) {
                    for (const auto& transition : nfa.states[state_id]->transitions) {
                        if (transition.first != '\0') { // 排除epsilon转换
                            input_chars.insert(transition.first);
                        }
                    }
                }
            }

            return input_chars;
        }

        // 检查NFA状态集合是否包含终态
        static bool contains_final_state(const NFA& nfa, const std::set<int>& states,
                                         int target_final_state)
        {
            for (int state_id : states) {
                if (state_id < static_cast<int>(nfa.states.size())
                    && (nfa.states[state_id]->is_final
                        || state_id == target_final_state)) {
                    return true;
                }
            }
            return false;
        }
    };

    // DFA匹配器
    class DFA_Matcher {
       public:
        static bool match(const DFA& dfa, const std::string& input)
        {
            if (dfa.start_state == -1) {
                return input.empty();
            }

            int current_state = dfa.start_state;

            for (char c : input) {
                auto it = dfa.states[current_state]->transitions.find(c);
                if (it == dfa.states[current_state]->transitions.end()) {
                    // 没有对应的转换，匹配失败
                    return false;
                }
                current_state = it->second;
            }

            // 检查最终状态是否为终态
            return current_state < static_cast<int>(dfa.states.size())
                   && dfa.states[current_state]->is_final;
        }
    };

    // 更新DFAState的ID计数器
    int DFAState::global_id_counter = 0;

} // namespace Regex
