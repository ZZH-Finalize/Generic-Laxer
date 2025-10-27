#pragma once

#include "nfa.hpp"
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iterator>
#include <map>
#include <set>
#include <exception>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <queue>

namespace regex {

    class dfa {
       private:
        using state_set = std::set<state::id_type>;
        using dfa_state_id = std::uint32_t;
        using dfa_transition_map = std::unordered_map<char, dfa_state_id>;

        struct dfa_state {
            state_set nfa_states;  // 对应NFA状态集合
            dfa_transition_map transitions;  // 转换表
            bool is_final;  // 是否为最终状态
        };

        std::vector<dfa_state> states;
        dfa_state_id start_state;
        std::vector<dfa_state_id> final_states;

        // 计算epsilon闭包
        state_set epsilon_closure(const state_set& states_set, const nfa& input_nfa) const {
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
                
                const auto& state = input_nfa.get_state(current_state);
                const auto& transitions = state.get_transitions();
                auto epsilon_it = transitions.find(static_cast<char>(255)); // epsilon转换用255表示
                if (epsilon_it != transitions.end()) {
                    for (auto epsilon_target : epsilon_it->second) {
                        if (epsilon_target < in_closure.size() && !in_closure[epsilon_target]) {
                            in_closure[epsilon_target] = true;
                            closure.insert(epsilon_target);
                            work_queue.push(epsilon_target);
                        }
                    }
                }
            }
            return closure;
        }

        state_set epsilon_closure(state::id_type state_id, const nfa& input_nfa) const {
            state_set single_set = {state_id};
            return epsilon_closure(single_set, input_nfa);
        }

        // 计算从给定状态集通过指定输入字符能到达的状态集的epsilon闭包
        state_set move(const state_set& states_set, char input, const nfa& input_nfa) const {
            state_set result;
            for (auto state_id : states_set) {
                if (state_id >= input_nfa.get_states().size()) continue;
                
                const auto& state = input_nfa.get_state(state_id);
                const auto& transitions = state.get_transitions();
                auto it = transitions.find(input);
                if (it != transitions.end()) {
                    for (auto target : it->second) {
                        result.insert(target);
                    }
                }
            }
            return result;
        }

        // 将NFA转换为DFA的核心算法
        void construct_dfa(const nfa& input_nfa) {
            if (input_nfa.get_states().empty()) {
                // 如果NFA没有状态，创建一个空的DFA
                dfa_state initial_state;
                initial_state.nfa_states = {};
                initial_state.is_final = false;
                states.push_back(initial_state);
                start_state = 0;
                return;
            }
            
            // 计算初始状态的epsilon闭包
            state_set initial_closure = epsilon_closure(input_nfa.get_start(), input_nfa);
            std::map<state_set, dfa_state_id> state_map;  // 映射NFA状态集到DFA状态ID
            std::vector<state_set> unmarked;  // 未标记的DFA状态
            
            // 创建初始DFA状态
            dfa_state initial_dfa_state;
            initial_dfa_state.nfa_states = initial_closure;
            initial_dfa_state.is_final = false;
            for (auto nfa_state : initial_closure) {
                if (nfa_state == input_nfa.get_final()) {
                    initial_dfa_state.is_final = true;
                    break;
                }
            }
            
            states.push_back(initial_dfa_state);
            state_map[initial_closure] = 0;
            start_state = 0;
            unmarked.push_back(initial_closure);
            
            // 子集构造算法
            while (!unmarked.empty()) {
                state_set current_set = unmarked.back();
                unmarked.pop_back();
                
                dfa_state_id current_id = state_map[current_set];
                dfa_state& current_dfa_state = states[current_id];
                
                // 尝试所有可能的输入字符
                std::set<char> input_chars;
                for (auto nfa_state_id : current_set) {
                    if (nfa_state_id >= input_nfa.get_states().size()) continue;
                    
                    const auto& nfa_state = input_nfa.get_state(nfa_state_id);
                    const auto& transitions = nfa_state.get_transitions();
                    for (const auto& [input, targets] : transitions) {
                        if (input != static_cast<char>(255)) { // 不包括epsilon转换
                            input_chars.insert(input);
                        }
                    }
                }
                
                // 对每个输入字符计算下一个状态
                for (char input_char : input_chars) {
                    state_set next_set = epsilon_closure(move(current_set, input_char, input_nfa), input_nfa);
                    if (next_set.empty()) continue;
                    
                    dfa_state_id next_id;
                    auto it = state_map.find(next_set);
                    if (it == state_map.end()) {
                        // 创建新的DFA状态
                        dfa_state new_dfa_state;
                        new_dfa_state.nfa_states = next_set;
                        new_dfa_state.is_final = false;
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
                    
                    // 添加转换
                    current_dfa_state.transitions[input_char] = next_id;
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

        explicit dfa(const std::string_view exp): dfa(nfa::from(exp))
        {
        }

        static dfa from(const nfa& nfa)
        {
            return dfa(nfa);
        }

        static dfa from(std::string_view exp)
        {
            return dfa(nfa::from(exp));
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
                auto it = current_dfa_state.transitions.find(c);
                
                if (it == current_dfa_state.transitions.end()) {
                    // 没有对应的转换，匹配失败
                    return false;
                }
                
                current_state = it->second;
            }

            // 检查最终状态是否为接受状态
            return std::find(final_states.begin(), final_states.end(), current_state) != final_states.end();
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
            return std::find(final_states.begin(), final_states.end(), start_state) != final_states.end();
        }
    };

} // namespace regex
