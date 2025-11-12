#pragma once

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

#include "nfa.hpp"
#include "final_state.hpp"

namespace regex {
    class dfa {
       private:
        class state: public basic_state<nfa::state::id_t> {
           public:
            // NFA状态集类型
            using closure = nfa::closure;

           private:
            // 对应NFA状态集合
            closure nfa_states;

            // 是否为最终状态
            bool final;

           public:
            // 构造函数初始化转换表为无效值
            explicit state(const closure& nfa_states, bool is_final = false)
                : nfa_states(nfa_states), final(is_final)
            {
                std::fill(this->transition_map.begin(), this->transition_map.end(),
                          std::numeric_limits<id_t>::max());
            }

            inline void set_final(bool is_final)
            {
                this->final = is_final;
            }

            inline bool is_final(void) const noexcept
            {
                return this->final;
            }

            inline const closure& get_closure(void) const
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
        friend class builder;

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
