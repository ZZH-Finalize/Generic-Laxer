#pragma once

#include "nfa.hpp"
#include <format>
#include <iterator>
#include <set>
#include <exception>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include <map>

namespace regex {

    class dfa {
       private:
        class state {
           public:
            // DFA状态id采用和NFA状态id相同类型, 但二者是是两个不同的概念
            using id_t = nfa::state::id_t;

            // NFA状态集类型
            using state_set_t = std::set<id_t>;

            // DFA状态转换表类型
            using transition_map_t =
                std::array<id_t, std::tuple_size_v<nfa::state::transition_map_t>>;

           private:
            // 对应NFA状态集合
            state_set_t nfa_states;

            // DFA状态转换表
            transition_map_t transitions;

            // 是否为最终状态
            bool final;

           public:
            // 构造函数初始化转换表为无效值
            explicit state(const state_set_t& nfa_states, bool is_final = false)
                : nfa_states(nfa_states), final(is_final)
            {
                std::fill(transitions.begin(), transitions.end(),
                          std::numeric_limits<id_t>::max());
            }

            void set_final(bool is_final)
            {
                this->final = is_final;
            }

            bool is_final(void) const
            {
                return this->final;
            }

            void set_transition(char ch, id_t id)
            {
                this->transitions[ch] = id;
            }

            id_t get_transition(char ch) const
            {
                return this->transitions[ch];
            }

            const transition_map_t& get_transition_map(void) const
            {
                return this->transitions;
            }
        };

        std::vector<state> states;
        state::id_t start_state;
        std::vector<state::id_t> final_states;

        // 默认构造函数，用于builder类创建实例
        dfa(): start_state(0)
        {
        }

        state::id_t add_state(const state::state_set_t& nfa_states, bool is_final = false)
        {
            this->states.emplace_back(nfa_states, is_final);

            return this->states.size() - 1;
        }

       public:
        friend dfa to_dfa(const nfa& nfa);
        friend dfa minimize(const dfa& dfa); // 添加友元函数用于最小化

        // 匹配算法：检查字符串是否与DFA匹配
        bool match(std::string_view str) const
        {
            if (this->states.empty()) {
                return false; // 没有状态，无法匹配
            }

            state::id_t current_state = this->start_state;

            for (char c : str) {
                // 边界检查：确保current_state在有效范围内
                if (current_state >= this->states.size()) {
                    return false; // 状态越界，匹配失败
                }

                const auto& current_dfa_state = this->states[current_state];
                // 检查转换是否有效（使用特殊值表示无效转换）
                state::id_t next_state = current_dfa_state.get_transition(c);
                if (next_state == std::numeric_limits<state::id_t>::max()) {
                    // 没有对应的转换，匹配失败
                    return false;
                }

                current_state = next_state;
            }

            // 检查最终状态是否为接受状态
            return std::find(this->final_states.begin(), this->final_states.end(),
                             current_state)
                   != this->final_states.end();
        }

        // 检查是否匹配空字符串
        bool match_empty() const
        {
            if (this->states.empty()) {
                return false;
            }
            // 边界检查：确保this->start_state在有效范围内
            if (this->start_state >= this->states.size()) {
                return false;
            }
            return std::find(this->final_states.begin(), this->final_states.end(),
                             this->start_state)
                   != this->final_states.end();
        }

        // 查找匹配：检查输入字符串中是否存在匹配模式的子串
        // 返回匹配的起始位置，-1表示未找到匹配
        int find_match(std::string_view str) const
        {
            if (this->states.empty()) {
                return -1; // 没有状态，无法匹配
            }

            // 尝试从每个位置开始匹配
            for (size_t start_pos = 0; start_pos < str.length(); ++start_pos) {
                state::id_t current_state = this->start_state;

                // 从当前位置开始尝试匹配
                for (size_t i = start_pos; i < str.length(); ++i) {
                    char c = str[i];

                    // 边界检查：确保current_state在有效范围内
                    if (current_state >= this->states.size()) {
                        break; // 状态越界，此路径匹配失败
                    }

                    const auto& current_dfa_state = this->states[current_state];
                    // 检查转换是否有效（使用特殊值表示无效转换）
                    state::id_t next_state = current_dfa_state.get_transition(c);
                    if (next_state == std::numeric_limits<state::id_t>::max()) {
                        // 没有对应的转换，匹配失败
                        break;
                    }

                    current_state = next_state;

                    // 检查当前位置是否为接受状态
                    if (std::find(this->final_states.begin(), this->final_states.end(),
                                  current_state)
                        != this->final_states.end()) {
                        // 找到匹配，返回起始位置
                        return static_cast<int>(start_pos);
                    }
                }
            }

            // 没有找到匹配
            return -1;
        }

        const auto& get_final_states(void) const
        {
            return this->final_states;
        }

        // 获取DFA的状态数量
        size_t get_state_count(void) const
        {
            return this->states.size();
        }

       private:
        class builder {
           private:
            using state_set_t = std::set<nfa::state::id_t>;
            using transition_map_t =
                std::array<nfa::state::id_t,
                           std::tuple_size_v<nfa::state::transition_map_t>>;

            // 计算epsilon闭包
            static state_set_t epsilon_closure(const state_set_t& states_set,
                                               const nfa& input_nfa)
            {
                state_set_t closure = states_set;
                std::vector<bool> in_closure(input_nfa.get_states().size(), false);

                // 标记初始状态在闭包中
                for (auto state_id : states_set) {
                    if (state_id < in_closure.size()) {
                        in_closure[state_id] = true;
                    }
                }

                // 使用队列进行BFS遍历，避免无限循环
                std::queue<nfa::state::id_t> work_queue;
                for (auto state_id : states_set) {
                    work_queue.push(state_id);
                }

                while (!work_queue.empty()) {
                    nfa::state::id_t current_state = work_queue.front();
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

            static state_set_t epsilon_closure(nfa::state::id_t state_id,
                                               const nfa& input_nfa)
            {
                state_set_t single_set = {state_id};
                return builder::epsilon_closure(single_set, input_nfa);
            }

            // 计算从给定状态集通过指定输入字符能到达的状态集的epsilon闭包
            static state_set_t move(const state_set_t& states_set, char input,
                                    const nfa& input_nfa)
            {
                state_set_t result;
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

           public:
            // 将NFA转换为DFA的核心算法
            static dfa build(const nfa& input_nfa)
            {
                dfa result_dfa;

                if (input_nfa.get_states().empty()) {
                    // 如果NFA没有状态，创建一个空的DFA
                    result_dfa.start_state = result_dfa.add_state({});
                    return result_dfa;
                }

                // 计算初始状态的epsilon闭包
                state_set_t initial_closure =
                    builder::epsilon_closure(input_nfa.get_start(), input_nfa);
                std::map<state_set_t, typename dfa::state::id_t>
                    state_map;                     // 映射NFA状态集到DFA状态ID
                std::vector<state_set_t> unmarked; // 未标记的DFA状态

                // 创建初始DFA状态
                bool is_final          = initial_closure.contains(input_nfa.get_final());
                result_dfa.start_state = result_dfa.add_state(initial_closure, is_final);

                state_map[initial_closure] = 0;

                unmarked.push_back(initial_closure);

                // 子集构造算法
                while (not unmarked.empty()) {
                    state_set_t current_set = unmarked.back();
                    unmarked.pop_back();

                    typename dfa::state::id_t current_id = state_map[current_set];

                    // 尝试所有可能的输入字符
                    std::set<char> input_chars;
                    for (auto nfa_state_id : current_set) {
                        if (nfa_state_id >= input_nfa.get_states().size()) continue;

                        const auto& nfa_state   = input_nfa.get_state(nfa_state_id);
                        const auto& transitions = nfa_state.get_transition_map();
                        for (int input_idx = 0; input_idx < transitions.size();
                             input_idx++) {
                            if (not transitions[input_idx].empty()) {
                                input_chars.insert(static_cast<char>(input_idx));
                            }
                        }
                    }

                    // 对每个输入字符计算下一个状态
                    for (char input_char : input_chars) {
                        state_set_t next_set = builder::epsilon_closure(
                            builder::move(current_set, input_char, input_nfa), input_nfa);
                        if (next_set.empty()) continue;

                        typename dfa::state::id_t next_id;
                        auto it = state_map.find(next_set);
                        if (it == state_map.end()) {
                            // 创建新的DFA状态

                            next_id = result_dfa.add_state(
                                next_set, next_set.contains(input_nfa.get_final()));
                            state_map[next_set] = next_id;
                            unmarked.push_back(next_set);
                        } else {
                            next_id = it->second;
                        }

                        // 添加转换 - 修复：每次都重新获取引用以避免引用失效
                        result_dfa.states[current_id].set_transition(input_char, next_id);
                    }
                }

                // 记录最终状态
                for (dfa::state::id_t i = 0; i < result_dfa.states.size(); i++) {
                    if (result_dfa.states[i].is_final()) {
                        result_dfa.final_states.push_back(i);
                    }
                }

                return result_dfa;
            }

            // DFA最小化算法 - 使用Hopcroft算法
            static dfa minimize(const dfa& input_dfa)
            {
                if (input_dfa.states.empty()) {
                    return input_dfa;
                }

                // 初始化等价类划分：将状态分为最终状态和非最终状态
                std::vector<std::set<dfa::state::id_t>> partition;
                std::set<dfa::state::id_t> final_state_set(input_dfa.final_states.begin(),
                                                           input_dfa.final_states.end());

                std::set<dfa::state::id_t> non_final_states;
                for (dfa::state::id_t i = 0; i < input_dfa.states.size(); i++) {
                    if (final_state_set.find(i) == final_state_set.end()) {
                        non_final_states.insert(i);
                    }
                }

                // 添加最终状态集合（如果非空）
                if (!final_state_set.empty()) {
                    partition.push_back(final_state_set);
                }

                // 添加非最终状态集合（如果非空）
                if (!non_final_states.empty()) {
                    partition.push_back(non_final_states);
                }

                // 获取DFA中实际使用的字符
                std::set<char> alphabet;
                for (const auto& state : input_dfa.states) {
                    const auto& transition_map = state.get_transition_map();
                    for (int i = 0; i < transition_map.size(); ++i) {
                        if (transition_map[i]
                            != std::numeric_limits<dfa::state::id_t>::max()) {
                            alphabet.insert(static_cast<char>(i));
                        }
                    }
                }

                bool changed = true;
                while (changed) {
                    changed = false;
                    std::vector<std::set<dfa::state::id_t>> new_partition;

                    for (const auto& block : partition) {
                        if (block.size() <= 1) {
                            // 单元素块无法进一步分割
                            new_partition.push_back(block);
                            continue;
                        }

                        // 尝试按输入字符分割当前块
                        bool split_occurred = false;
                        std::vector<std::set<dfa::state::id_t>> temp_partition;
                        // 初始化第一个子集，包含block的第一个元素
                        std::set<dfa::state::id_t> initial_subset;
                        initial_subset.insert(*block.begin());
                        temp_partition.push_back(initial_subset);

                        for (auto it = std::next(block.begin()); it != block.end();
                             ++it) {
                            dfa::state::id_t current_state = *it;
                            bool placed                    = false;

                            for (auto& temp_block : temp_partition) {
                                dfa::state::id_t representative =
                                    *temp_block.begin(); // 选一个代表状态
                                bool equivalent = true;

                                // 检查两个状态是否等价（对于所有输入字符，转换到的等价类相同）
                                for (char c : alphabet) {
                                    dfa::state::id_t current_next =
                                        input_dfa.states[current_state].get_transition(c);
                                    dfa::state::id_t rep_next =
                                        input_dfa.states[representative].get_transition(
                                            c);

                                    // 如果一个状态有转换而另一个没有，则不等价
                                    if ((current_next
                                         == std::numeric_limits<dfa::state::id_t>::max())
                                        != (rep_next
                                            == std::numeric_limits<
                                                dfa::state::id_t>::max())) {
                                        equivalent = false;
                                        break;
                                    }

                                    if (current_next
                                            != std::numeric_limits<
                                                dfa::state::id_t>::max()
                                        && rep_next
                                               != std::numeric_limits<
                                                   dfa::state::id_t>::max()) {
                                        // 找到这两个状态所属的等价类
                                        int current_class = -1, rep_class = -1;
                                        for (int i = 0; i < partition.size(); ++i) {
                                            if (partition[i].count(current_next))
                                                current_class = i;
                                            if (partition[i].count(rep_next))
                                                rep_class = i;
                                        }

                                        if (current_class != rep_class) {
                                            equivalent = false;
                                            break;
                                        }
                                    }
                                }

                                if (equivalent) {
                                    temp_block.insert(current_state);
                                    placed = true;
                                    break;
                                }
                            }

                            if (!placed) {
                                std::set<dfa::state::id_t> new_subset;
                                new_subset.insert(current_state);
                                temp_partition.push_back(new_subset);
                            }
                        }

                        if (temp_partition.size() > 1) {
                            changed        = true;
                            split_occurred = true;
                            for (const auto& new_block : temp_partition) {
                                if (!new_block.empty()) {
                                    new_partition.push_back(new_block);
                                }
                            }
                        } else {
                            new_partition.push_back(block);
                        }
                    }

                    if (changed) {
                        partition = new_partition;
                    }
                }

                // 创建最小化DFA
                dfa minimized_dfa;

                // 创建映射：原状态ID -> 最小化后状态ID
                std::map<dfa::state::id_t, dfa::state::id_t> state_mapping;

                for (dfa::state::id_t i = 0; i < partition.size(); i++) {
                    for (auto state_id : partition[i]) {
                        state_mapping[state_id] = i;
                    }
                }

                // 构建最小化的DFA状态
                for (dfa::state::id_t i = 0; i < partition.size(); i++) {
                    // 从等价类中任选一个状态来决定新状态的属性
                    dfa::state::id_t representative = *partition[i].begin();
                    bool is_final = input_dfa.states[representative].is_final();

                    // 创建新状态并添加到最小化DFA中
                    dfa::state::state_set_t
                        dummy_set; // 构造函数需要，但对最小化DFA不重要
                    dfa::state::id_t new_state_id =
                        minimized_dfa.add_state(dummy_set, is_final);

                    // 设置转换
                    for (char c : alphabet) {
                        dfa::state::id_t old_next =
                            input_dfa.states[representative].get_transition(c);
                        if (old_next != std::numeric_limits<dfa::state::id_t>::max()) {
                            dfa::state::id_t new_next = state_mapping[old_next];
                            minimized_dfa.states[new_state_id].set_transition(c,
                                                                              new_next);
                        }
                    }
                }

                // 设置起始状态
                minimized_dfa.start_state = state_mapping[input_dfa.start_state];

                // 设置最终状态列表
                for (auto final_id : input_dfa.final_states) {
                    dfa::state::id_t new_final_id = state_mapping[final_id];
                    if (std::find(minimized_dfa.final_states.begin(),
                                  minimized_dfa.final_states.end(), new_final_id)
                        == minimized_dfa.final_states.end()) {
                        minimized_dfa.final_states.push_back(new_final_id);
                    }
                }

                return minimized_dfa;
            }
        };
    };

} // namespace regex
