#pragma once

#include "nfa.hpp"
#include <format>
#include <iterator>
#include <optional>
#include <set>
#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include <map>

#include "final_state.hpp"

namespace regex {

    class dfa {
       private:
        class state {
           public:
            // DFA状态id采用和NFA状态id相同类型, 但二者是是两个不同的概念
            using id_t = nfa::state::id_t;

            // NFA状态集类型
            using closure = nfa::closure;

            // DFA状态转换表类型
            using transition_map_t =
                std::array<id_t, std::tuple_size_v<nfa::state::transition_map_t>>;

           private:
            // 对应NFA状态集合
            closure nfa_states;

            // DFA状态转换表
            transition_map_t transitions;

            // 是否为最终状态
            bool final;

           public:
            // 构造函数初始化转换表为无效值
            explicit state(const closure& nfa_states, bool is_final = false)
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

            const closure& get_closure(void) const
            {
                return this->nfa_states;
            }
        };

        using final_states_t = std::set<final_state>;

        std::vector<state> states;
        state::id_t start_state;
        final_states_t final_states;

        // 默认构造函数，用于builder类创建实例
        dfa(): start_state(0)
        {
        }

        state::id_t add_state(const state::closure& nfa_states, bool is_final = false)
        {
            this->states.emplace_back(nfa_states, is_final);

            return this->states.size() - 1;
        }

        void set_transition(state::id_t current, char ch, state::id_t to)
        {
            this->states[current].set_transition(ch, to);
        }

       public:
        using id_t = state::id_t;
        friend dfa to_dfa(const nfa& nfa);
        friend dfa minimize(const dfa& dfa); // 添加友元函数用于最小化

        inline static const state::id_t invalid_state =
            std::numeric_limits<state::id_t>::max();

        const auto& get_states(void) const
        {
            return this->states;
        }

        auto get_start_state(void) const
        {
            return this->start_state;
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

        // 匹配算法：检查字符串是否与DFA匹配
        std::optional<final_state> match(std::string_view str) const;

        // 检查是否匹配空字符串
        bool match_empty() const;

        // 查找匹配：检查输入字符串中是否存在匹配模式的子串
        // 返回匹配的起始位置，-1表示未找到匹配
        int find_match(std::string_view str) const;

       public:
        class builder {
           private:
            using closure = dfa::state::closure;
            using transition_map_t =
                std::array<nfa::state::id_t,
                           std::tuple_size_v<nfa::state::transition_map_t>>;

           public:
            // 计算epsilon闭包
            template<typename NFA>
            requires is_nfa<NFA>
            static dfa::builder::closure epsilon_closure(const closure& states_set,
                                                         const NFA& input_nfa)
            {
                closure closure = states_set;

                // 使用队列进行BFS遍历，避免无限循环
                std::queue<nfa::state::id_t> work_queue;
                for (auto state_id : states_set) {
                    work_queue.push(state_id);
                }

                while (not work_queue.empty()) {
                    nfa::state::id_t current_state = work_queue.front();
                    work_queue.pop();

                    const auto& state           = input_nfa.get_state(current_state);
                    const auto& epsilon_targets = state.get_epsilon_transition();

                    for (auto epsilon_target : epsilon_targets) {
                        closure.insert(epsilon_target);
                        work_queue.push(epsilon_target);
                    }
                }

                return closure;
            }

            template<typename NFA>
            requires is_nfa<NFA>
            static closure epsilon_closure(nfa::state::id_t state_id,
                                           const NFA& input_nfa)
            {
                closure single_set = {state_id};
                return builder::epsilon_closure(single_set, input_nfa);
            }

            template<typename NFA>
            requires is_nfa<NFA>
            static dfa::builder::closure move(const closure& states_set, char input,
                                              const NFA& input_nfa)
            {
                closure result;

                for (auto state_id : states_set) {
                    const auto& state          = input_nfa.get_state(state_id);
                    const auto& transition_map = state.get_transition_map();

                    for (auto target :
                         transition_map[static_cast<unsigned char>(input)]) {
                        result.insert(target);
                    }
                }

                return result;
            }

            template<typename NFA>
            requires is_nfa<NFA>
            static dfa build(const NFA& input_nfa)
            {
                dfa result_dfa;

                if (input_nfa.get_states().empty()) {
                    // 如果NFA没有状态，创建一个空的DFA
                    result_dfa.start_state = result_dfa.add_state({});
                    return result_dfa;
                }

                // 计算初始状态的epsilon闭包
                closure initial_closure =
                    builder::epsilon_closure(input_nfa.get_start(), input_nfa);
                std::map<closure, dfa::state::id_t> state_map; // 映射NFA状态集到DFA状态ID
                std::vector<closure> unmarked;                 // 未标记的DFA状态

                // 创建初始DFA状态
                bool is_final          = input_nfa.has_final(initial_closure);
                result_dfa.start_state = result_dfa.add_state(initial_closure, is_final);

                state_map[initial_closure] = result_dfa.start_state;

                unmarked.push_back(initial_closure);

                // 子集构造算法
                while (not unmarked.empty()) {
                    closure current_set = unmarked.back();
                    unmarked.pop_back();

                    dfa::state::id_t current_id = state_map[current_set];

                    // 尝试所有可能的输入字符
                    std::set<char> input_chars;
                    for (auto nfa_state_id : current_set) {
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
                        closure next_set = builder::epsilon_closure(
                            builder::move(current_set, input_char, input_nfa), input_nfa);
                        if (next_set.empty()) continue;

                        dfa::state::id_t next_id;
                        auto it = state_map.find(next_set);
                        if (it == state_map.end()) {
                            // 创建新的DFA状态
                            next_id             = result_dfa.add_state(next_set,
                                                                       input_nfa.has_final(next_set));
                            state_map[next_set] = next_id;
                            unmarked.push_back(next_set);
                        } else {
                            next_id = it->second;
                        }

                        // 添加转换
                        result_dfa.set_transition(current_id, input_char, next_id);
                    }
                }

                // 记录最终状态
                for (dfa::state::id_t i = 0; i < result_dfa.states.size(); i++) {
                    if (result_dfa.states[i].is_final()) {
                        final_state state(i);
                        const auto& closure = result_dfa.states[i].get_closure();

                        if constexpr (has_metadata<NFA>) {
                            state.set_metadata(input_nfa.get_metadata(closure));
                        }

                        result_dfa.final_states.insert(state);
                    }
                }

                return result_dfa;
            }

            // DFA最小化算法 - 使用Hopcroft算法
            static dfa minimize(const dfa& input_dfa);
        };
    };

} // namespace regex

template<>
struct std::formatter<regex::dfa>
{
    std::string direction;

    constexpr auto parse(std::format_parse_context& ctx)
    {
        auto it = ctx.begin();

        while (*it != '}') {
            this->direction.push_back(*it++);
        }

        // 检查方向字符串是否有效
        if (this->direction != "LR" and this->direction != "TB"
            and this->direction != "RL" and not this->direction.empty()) {
            return ctx.end();
        }

        return it;
    }

    auto format(const regex::dfa& dfa, std::format_context& ctx) const
    {
        std::string result = "```mermaid\n";

        result += "stateDiagram-v2\n";

        if (not this->direction.empty()) {
            result += std::format("direction {}\n", this->direction);
        }

        result += "\n";

        auto get_state_str = [&dfa](regex::dfa::id_t id) {
            auto& final_states = dfa.get_final_states();
            std::string state_str;

            if (id == dfa.get_start_state()) {
                // return std::string("[*]");
                state_str += "start_";
            }

            if (final_states.contains(regex::final_state(id))) {
                // return std::string("[*]");
                state_str += "final_";
            }

            if (not state_str.empty()) {
                return std::format("{}{}", state_str, id);
            }

            return std::format("state_{}", id);
        };

        auto& states = dfa.get_states();

        std::map<std::size_t, std::set<char>> merge_map;

        for (regex::dfa::id_t current_state_id = 0; current_state_id < states.size();
             current_state_id++) {
            auto& state     = states[current_state_id];
            auto& trans_map = state.get_transition_map();

            for (auto input_char = 0; input_char < trans_map.size(); input_char++) {
                auto to_state_id = trans_map[input_char];
                if (to_state_id == regex::dfa::invalid_state) {
                    continue;
                } else if (current_state_id == to_state_id) {
                    merge_map[current_state_id].insert(input_char);
                    continue;
                }

                const auto& from = get_state_str(current_state_id);
                const auto& to   = get_state_str(to_state_id);

                result += std::format("{} --> {} : {}\n", from, to,
                                      static_cast<char>(input_char));
            }
        }

        for (auto& [state_id, input_chars] : merge_map) {
            std::string state_str = get_state_str(state_id);
            std::string input_char;

            for (char ch : input_chars) {
                input_char += std::format("{},", ch);
            }

            input_char.pop_back();

            result += std::format("{} --> {} : {}\n", state_str, state_str, input_char);
        }

        result += "```";

        return std::format_to(ctx.out(), "{}", result);
    }
};
