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

        state::id_t add_state(const state::state_set_t& nfa_states, bool is_final = false)
        {
            this->states.emplace_back(nfa_states, is_final);

            return this->states.size() - 1;
        }

        // 计算epsilon闭包
        state::state_set_t epsilon_closure(const state::state_set_t& states_set,
                                           const nfa& input_nfa) const
        {
            state::state_set_t closure = states_set;
            std::vector<bool> in_closure(input_nfa.get_states().size(), false);

            // 标记初始状态在闭包中
            for (auto state_id : states_set) {
                if (state_id < in_closure.size()) {
                    in_closure[state_id] = true;
                }
            }

            // 使用队列进行BFS遍历，避免无限循环
            std::queue<state::id_t> work_queue;
            for (auto state_id : states_set) {
                work_queue.push(state_id);
            }

            while (!work_queue.empty()) {
                state::id_t current_state = work_queue.front();
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

        state::state_set_t epsilon_closure(state::id_t state_id,
                                           const nfa& input_nfa) const
        {
            state::state_set_t single_set = {state_id};
            return epsilon_closure(single_set, input_nfa);
        }

        // 计算从给定状态集通过指定输入字符能到达的状态集的epsilon闭包
        state::state_set_t move(const state::state_set_t& states_set, char input,
                                const nfa& input_nfa) const
        {
            state::state_set_t result;
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
                this->start_state = this->add_state({});
                return;
            }

            // 计算初始状态的epsilon闭包
            state::state_set_t initial_closure =
                epsilon_closure(input_nfa.get_start(), input_nfa);
            std::map<state::state_set_t, state::id_t>
                state_map;                            // 映射NFA状态集到DFA状态ID
            std::vector<state::state_set_t> unmarked; // 未标记的DFA状态

            // 创建初始DFA状态
            bool is_final     = initial_closure.contains(input_nfa.get_final());
            this->start_state = this->add_state(initial_closure, is_final);

            state_map[initial_closure] = 0;

            unmarked.push_back(initial_closure);

            // 子集构造算法
            while (not unmarked.empty()) {
                state::state_set_t current_set = unmarked.back();
                unmarked.pop_back();

                state::id_t current_id = state_map[current_set];

                // 尝试所有可能的输入字符
                std::set<char> input_chars;
                for (auto nfa_state_id : current_set) {
                    if (nfa_state_id >= input_nfa.get_states().size()) continue;

                    const auto& nfa_state   = input_nfa.get_state(nfa_state_id);
                    const auto& transitions = nfa_state.get_transition_map();
                    for (int input_idx = 0; input_idx < transitions.size(); input_idx++) {
                        if (not transitions[input_idx].empty()) {
                            input_chars.insert(static_cast<char>(input_idx));
                        }
                    }
                }

                // 对每个输入字符计算下一个状态
                for (char input_char : input_chars) {
                    state::state_set_t next_set = epsilon_closure(
                        move(current_set, input_char, input_nfa), input_nfa);
                    if (next_set.empty()) continue;

                    state::id_t next_id;
                    auto it = state_map.find(next_set);
                    if (it == state_map.end()) {
                        // 创建新的DFA状态

                        next_id = this->add_state(
                            next_set, next_set.contains(input_nfa.get_final()));
                        state_map[next_set] = next_id;
                        unmarked.push_back(next_set);
                    } else {
                        next_id = it->second;
                    }

                    // 添加转换 - 修复：每次都重新获取引用以避免引用失效
                    this->states[current_id].set_transition(input_char, next_id);
                }
            }

            // 记录最终状态
            for (state::id_t i = 0; i < this->states.size(); ++i) {
                if (this->states[i].is_final()) {
                    this->final_states.push_back(i);
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

        void minimize()
        {
        }

        // 获取DFA的状态数量
        size_t get_state_count() const
        {
            return this->states.size();
        }
    };

} // namespace regex
