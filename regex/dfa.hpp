#pragma once

#include "nfa.hpp"
#include <format>
#include <iterator>
#include <set>
#include <exception>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <vector>
#include <queue>
#include <limits>
#include <algorithm>

namespace regex {

    class dfa {
       private:
        using state_set          = std::set<state::id_type>;
        using dfa_state_id       = state::id_type;
        using dfa_transition_map = std::array<dfa_state_id, 256>;

        struct dfa_state
        {
            state_set nfa_states;           // 对应NFA状态集合
            dfa_transition_map transitions; // 转换表
            bool is_final;                  // 是否为最终状态

            // 构造函数初始化转换表为无效值
            dfa_state(): transitions(), nfa_states(), is_final(false)
            {
                std::fill(transitions.begin(), transitions.end(),
                          std::numeric_limits<dfa_state_id>::max());
            }
        };

        std::vector<dfa_state> states;
        dfa_state_id start_state;
        std::vector<dfa_state_id> final_states;

        // 计算epsilon闭包
        state_set epsilon_closure(const state_set& states_set, const nfa& input_nfa) const
        {
            state_set closure = states_set;
            std::vector<bool> in_closure(input_nfa.get_states().size(), false);

            // 标记初始状态在闭包中
            for (auto state_id : states_set) {
                if (state_id < in_closure.size()) {
                    in_closure[state_id] = true;
                }
            }

            // 使用队列进行BFS遍历，避免无限循环
            std::queue<state::id_type> work_queue;
            for (auto state_id : states_set) {
                work_queue.push(state_id);
            }

            while (!work_queue.empty()) {
                state::id_type current_state = work_queue.front();
                work_queue.pop();

                if (current_state >= input_nfa.get_states().size()) continue;

                const auto& state           = input_nfa.get_state(current_state);
                const auto& epsilon_targets = state.get_epsilon_transition();
                for (auto epsilon_target : epsilon_targets) {
                    if (epsilon_target < in_closure.size()
                        and not in_closure[epsilon_target]) {
                        in_closure[epsilon_target] = true;

                        closure.insert(epsilon_target);
                        work_queue.push(epsilon_target);
                    }
                }
            }
            return closure;
        }

        state_set epsilon_closure(state::id_type state_id, const nfa& input_nfa) const
        {
            state_set single_set = {state_id};
            return epsilon_closure(single_set, input_nfa);
        }

        // 计算从给定状态集通过指定输入字符能到达的状态集的epsilon闭包
        state_set move(const state_set& states_set, char input,
                       const nfa& input_nfa) const
        {
            state_set result;
            for (auto state_id : states_set) {
                if (state_id >= input_nfa.get_states().size()) continue;

                const auto& state          = input_nfa.get_state(state_id);
                const auto& transition_map = state.get_transition_map();

                if (input >= 0) {
                    for (auto target :
                         transition_map[static_cast<unsigned char>(input)]) {
                        result.insert(target);
                    }
                }
            }

            return result;
        }

        // 将NFA转换为DFA的核心算法
        void construct_dfa(const nfa& input_nfa)
        {
            if (input_nfa.get_states().empty()) {
                // 如果NFA没有状态，创建一个空的DFA
                dfa_state initial_state;
                initial_state.nfa_states = {};
                initial_state.is_final   = false;
                states.push_back(initial_state);
                start_state = 0;
                return;
            }

            // 计算初始状态的epsilon闭包
            state_set initial_closure = epsilon_closure(input_nfa.get_start(), input_nfa);
            std::map<state_set, dfa_state_id> state_map; // 映射NFA状态集到DFA状态ID
            std::vector<state_set> unmarked;             // 未标记的DFA状态

            // 创建初始DFA状态
            dfa_state initial_dfa_state;
            initial_dfa_state.nfa_states = initial_closure;
            initial_dfa_state.is_final   = false;
            for (auto nfa_state : initial_closure) {
                if (nfa_state == input_nfa.get_final()) {
                    initial_dfa_state.is_final = true;
                    break;
                }
            }

            states.push_back(initial_dfa_state);
            state_map[initial_closure] = 0;
            start_state                = 0;
            unmarked.push_back(initial_closure);

            // 子集构造算法
            while (!unmarked.empty()) {
                state_set current_set = unmarked.back();
                unmarked.pop_back();

                dfa_state_id current_id = state_map[current_set];

                // 尝试所有可能的输入字符
                std::set<char> input_chars;
                for (auto nfa_state_id : current_set) {
                    if (nfa_state_id >= input_nfa.get_states().size()) continue;

                    const auto& nfa_state   = input_nfa.get_state(nfa_state_id);
                    const auto& transitions = nfa_state.get_transition_map();
                    for (int input_idx = 0; input_idx < 256; ++input_idx) {
                        if (!transitions[input_idx].empty()) {
                            input_chars.insert(static_cast<char>(input_idx));
                        }
                    }
                }

                // 对每个输入字符计算下一个状态
                for (char input_char : input_chars) {
                    state_set next_set = epsilon_closure(
                        move(current_set, input_char, input_nfa), input_nfa);
                    if (next_set.empty()) continue;

                    dfa_state_id next_id;
                    auto it = state_map.find(next_set);
                    if (it == state_map.end()) {
                        // 创建新的DFA状态
                        dfa_state new_dfa_state;
                        new_dfa_state.nfa_states = next_set;
                        new_dfa_state.is_final   = false;
                        for (auto nfa_state : next_set) {
                            if (nfa_state == input_nfa.get_final()) {
                                new_dfa_state.is_final = true;
                                break;
                            }
                        }

                        next_id = states.size();
                        states.push_back(new_dfa_state);
                        state_map[next_set] = next_id;
                        unmarked.push_back(next_set);
                    } else {
                        next_id = it->second;
                    }

                    // 添加转换 - 修复：每次都重新获取引用以避免引用失效
                    states[current_id]
                        .transitions[static_cast<unsigned char>(input_char)] = next_id;
                }
            }

            // 记录最终状态
            for (dfa_state_id i = 0; i < states.size(); ++i) {
                if (states[i].is_final) {
                    final_states.push_back(i);
                }
            }
        }

       public:
        explicit dfa(const class nfa& nfa)
        {
            construct_dfa(nfa);
        }

        explicit dfa(const std::string_view exp): dfa(nfa::builder::build(exp))
        {
        }

        static dfa from(const nfa& nfa)
        {
            return dfa(nfa);
        }

        static dfa from(std::string_view exp)
        {
            return dfa(nfa::builder::build(exp));
        }

        // 匹配算法：检查字符串是否与DFA匹配
        bool match(std::string_view str) const
        {
            if (states.empty()) {
                return false; // 没有状态，无法匹配
            }

            dfa_state_id current_state = start_state;

            for (char c : str) {
                // 边界检查：确保current_state在有效范围内
                if (current_state >= states.size()) {
                    return false; // 状态越界，匹配失败
                }

                const auto& current_dfa_state = states[current_state];
                // 检查转换是否有效（使用特殊值表示无效转换）
                dfa_state_id next_state =
                    current_dfa_state.transitions[static_cast<unsigned char>(c)];
                if (next_state == std::numeric_limits<dfa_state_id>::max()) {
                    // 没有对应的转换，匹配失败
                    return false;
                }

                current_state = next_state;
            }

            // 检查最终状态是否为接受状态
            return std::find(final_states.begin(), final_states.end(), current_state)
                   != final_states.end();
        }

        // 检查是否匹配空字符串
        bool match_empty() const
        {
            if (states.empty()) {
                return false;
            }
            // 边界检查：确保start_state在有效范围内
            if (start_state >= states.size()) {
                return false;
            }
            return std::find(final_states.begin(), final_states.end(), start_state)
                   != final_states.end();
        }

        // 查找匹配：检查输入字符串中是否存在匹配模式的子串
        // 返回匹配的起始位置，-1表示未找到匹配
        int find_match(std::string_view str) const
        {
            if (states.empty()) {
                return -1; // 没有状态，无法匹配
            }

            // 尝试从每个位置开始匹配
            for (size_t start_pos = 0; start_pos < str.length(); ++start_pos) {
                dfa_state_id current_state = start_state;

                // 从当前位置开始尝试匹配
                for (size_t i = start_pos; i < str.length(); ++i) {
                    char c = str[i];

                    // 边界检查：确保current_state在有效范围内
                    if (current_state >= states.size()) {
                        break; // 状态越界，此路径匹配失败
                    }

                    const auto& current_dfa_state = states[current_state];
                    // 检查转换是否有效（使用特殊值表示无效转换）
                    dfa_state_id next_state =
                        current_dfa_state.transitions[static_cast<unsigned char>(c)];
                    if (next_state == std::numeric_limits<dfa_state_id>::max()) {
                        // 没有对应的转换，匹配失败
                        break;
                    }

                    current_state = next_state;

                    // 检查当前位置是否为接受状态
                    if (std::find(final_states.begin(), final_states.end(), current_state)
                        != final_states.end()) {
                        // 找到匹配，返回起始位置
                        return static_cast<int>(start_pos);
                    }
                }
            }

            // 没有找到匹配
            return -1;
        }

        // DFA最小化函数 - 使用Hopcroft算法
        void minimize()
        {
            if (states.empty()) {
                return; // 空DFA无需最小化
            }

            // 获取所有输入字符
            std::set<char> input_chars;
            for (const auto& dfa_state : states) {
                for (size_t i = 0; i < dfa_state.transitions.size(); ++i) {
                    if (dfa_state.transitions[i]
                        != std::numeric_limits<dfa_state_id>::max()) {
                        input_chars.insert(static_cast<char>(i));
                    }
                }
            }

            // 初始化等价类划分：将状态分为接受状态和非接受状态
            std::vector<std::set<dfa_state_id>> partition;
            std::set<dfa_state_id> final_state_set(final_states.begin(),
                                                   final_states.end());

            std::set<dfa_state_id> non_final_states;
            for (dfa_state_id i = 0; i < states.size(); ++i) {
                if (final_state_set.find(i) == final_state_set.end()) {
                    non_final_states.insert(i);
                }
            }

            // 添加接受状态集合（如果非空）
            if (!final_state_set.empty()) {
                partition.push_back(final_state_set);
            }

            // 添加非接受状态集合（如果非空）
            if (!non_final_states.empty()) {
                partition.push_back(non_final_states);
            }

            bool changed = true;
            while (changed) {
                changed = false;

                std::vector<std::set<dfa_state_id>> new_partition;

                // 对每个等价类进行细化
                for (const auto& equiv_class : partition) {
                    if (equiv_class.size() <= 1) {
                        // 单元素集合不能再分割
                        new_partition.push_back(equiv_class);
                        continue;
                    }

                    // 使用每个输入字符对等价类进行细分
                    std::map<std::vector<dfa_state_id>, std::vector<dfa_state_id>>
                        signatures;

                    for (dfa_state_id state_id : equiv_class) {
                        // 为当前状态创建签名：对于每个输入字符，记录转换到的目标等价类
                        std::vector<dfa_state_id> signature;
                        for (char input : input_chars) {
                            // 找到当前状态下通过输入字符转换到的状态
                            dfa_state_id target_state =
                                states[state_id]
                                    .transitions[static_cast<unsigned char>(input)];
                            if (target_state
                                != std::numeric_limits<dfa_state_id>::max()) {
                                // 找到目标状态所属的等价类
                                dfa_state_id target_class_id = 0;
                                for (size_t i = 0; i < partition.size(); ++i) {
                                    if (partition[i].find(target_state)
                                        != partition[i].end()) {
                                        target_class_id = static_cast<dfa_state_id>(i);
                                        break;
                                    }
                                }
                                signature.push_back(target_class_id);
                            } else {
                                // 如果没有转换，使用特殊值表示
                                signature.push_back(
                                    std::numeric_limits<dfa_state_id>::max());
                            }
                        }

                        // 使用签名对状态进行分组
                        signatures[signature].push_back(state_id);
                    }

                    // 检查是否需要分割当前等价类
                    if (signatures.size() > 1) {
                        changed = true;
                        for (const auto& [_, state_group] : signatures) {
                            std::set<dfa_state_id> new_class(state_group.begin(),
                                                             state_group.end());
                            new_partition.push_back(new_class);
                        }
                    } else {
                        new_partition.push_back(equiv_class);
                    }
                }

                partition = new_partition;
            }

            // 创建新的最小化DFA
            std::vector<dfa_state> new_states;
            std::vector<dfa_state_id> new_final_states;
            dfa_state_id new_start_state = 0;

            // 为每个等价类创建一个新状态
            std::map<dfa_state_id, dfa_state_id> old_to_new; // 旧状态ID到新状态ID的映射

            for (size_t i = 0; i < partition.size(); ++i) {
                const auto& equiv_class = partition[i];

                // 创建新状态，使用等价类中第一个状态的信息
                dfa_state new_state;
                dfa_state_id representative = *equiv_class.begin();

                new_state.nfa_states = states[representative].nfa_states;
                new_state.is_final   = states[representative].is_final;

                // 构建新状态的转换表
                for (char input : input_chars) {
                    dfa_state_id target_state =
                        states[representative]
                            .transitions[static_cast<unsigned char>(input)];
                    if (target_state != std::numeric_limits<dfa_state_id>::max()) {
                        // 找到目标状态所属的等价类ID
                        dfa_state_id target_class_id = 0;
                        for (size_t j = 0; j < partition.size(); ++j) {
                            if (partition[j].find(target_state) != partition[j].end()) {
                                target_class_id = static_cast<dfa_state_id>(j);
                                break;
                            }
                        }

                        new_state.transitions[static_cast<unsigned char>(input)] =
                            target_class_id;
                    }
                }

                new_states.push_back(new_state);

                // 记录映射关系
                for (dfa_state_id old_id : equiv_class) {
                    old_to_new[old_id] = static_cast<dfa_state_id>(new_states.size() - 1);
                }

                // 检查这个等价类是否包含原起始状态
                if (equiv_class.find(start_state) != equiv_class.end()) {
                    new_start_state = static_cast<dfa_state_id>(new_states.size() - 1);
                }

                // 检查这个等价类是否包含最终状态
                bool has_final = false;
                for (dfa_state_id old_id : equiv_class) {
                    if (std::find(final_states.begin(), final_states.end(), old_id)
                        != final_states.end()) {
                        has_final = true;
                        break;
                    }
                }
                if (has_final) {
                    new_final_states.push_back(
                        static_cast<dfa_state_id>(new_states.size() - 1));
                }
            }

            // 更新DFA
            states       = std::move(new_states);
            start_state  = new_start_state;
            final_states = std::move(new_final_states);
        }

        // 获取DFA的状态数量
        size_t get_state_count() const
        {
            return states.size();
        }
    };

} // namespace regex
