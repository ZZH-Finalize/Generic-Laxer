#pragma once

#include <format>
#include <optional>
#include <set>
#include <string_view>
#include <vector>
#include <limits>
#include <algorithm>

#include "basic_fa.hpp"
#include "regex_concepts.hpp"

namespace regex {

    class dfa_state: public basic_state<std::uint32_t> {
       private:
        // 对应NFA状态集合
        closure_t nfa_states;

        // 是否为最终状态
        bool final;

       public:
        // 构造函数初始化转换表为无效值
        explicit dfa_state(const closure_t& nfa_states = {}, bool is_final = false)
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

        inline const closure_t& get_closure(void) const
        {
            return this->nfa_states;
        }
    };

    // DFA必然是多终态的, 因此终态类型必须是容器类型
    template<typename final_state_t>
    requires is_fa_final_state_container<final_state_t>
    class basic_dfa: public basic_fa<dfa_state, final_state_t> {
       public:
        friend class builder;
        friend struct std::formatter<basic_dfa>;
        using final_state_id_t = final_state_t::value_type;

        // 匹配算法：检查字符串是否与DFA匹配
        std::optional<final_state_id_t> match(std::string_view str) const
        {
            if (this->states.empty()) {
                return std::nullopt; // 没有状态，无法匹配
            }

            id_t current_state = this->get_start();

            for (char c : str) {
                // 边界检查：确保current_state在有效范围内
                if (current_state >= this->states.size()) {
                    return std::nullopt; // 状态越界，匹配失败
                }

                const auto& current_dfa_state = this->states[current_state];
                // 检查转换是否有效（使用特殊值表示无效转换）
                id_t next_state = current_dfa_state.get_transition(c);
                if (next_state == basic_dfa::invalid_state) {
                    // 没有对应的转换，匹配失败
                    return std::nullopt;
                }

                current_state = next_state;
            }

            const auto& find_state = this->get_final().find(current_state);

            // 检查最终状态是否为接受状态
            if (find_state != this->get_final().end()) {
                return *find_state;
            }

            return std::nullopt;
        }

        // 检查是否匹配空字符串
        bool match_empty() const
        {
            if (this->states.empty()) {
                return false;
            }
            // 边界检查：确保this->get_start()在有效范围内
            if (this->get_start() >= this->states.size()) {
                return false;
            }
            return this->get_final().contains(this->get_start());
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
                id_t current_state = this->get_start();

                // 从当前位置开始尝试匹配
                for (size_t i = start_pos; i < str.length(); ++i) {
                    char c = str[i];

                    // 边界检查：确保current_state在有效范围内
                    if (current_state >= this->states.size()) {
                        break; // 状态越界，此路径匹配失败
                    }

                    const auto& current_dfa_state = this->states[current_state];
                    // 检查转换是否有效（使用特殊值表示无效转换）
                    id_t next_state = current_dfa_state.get_transition(c);
                    if (next_state == basic_dfa::invalid_state) {
                        // 没有对应的转换，匹配失败
                        break;
                    }

                    current_state = next_state;

                    // 检查当前位置是否为接受状态
                    if (this->get_final().contains(current_state)) {
                        // 找到匹配，返回起始位置
                        return static_cast<int>(start_pos);
                    }
                }
            }

            // 没有找到匹配
            return -1;
        }
    };

} // namespace regex
