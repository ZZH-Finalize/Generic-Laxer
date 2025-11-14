#pragma once

#include <exception>
#include <format>

#include "basic_fa.hpp"
#include "basic_state.hpp"

namespace regex {
    class nfa_state: public basic_state<std::vector<id_t>> {
       private:
        transition_map_item_t epsilon_transitions;

       public:
        inline void add_epsilon_transition(id_t to)
        {
            this->epsilon_transitions.push_back(to);
        }

        inline const auto& get_epsilon_transition(void) const
        {
            return this->epsilon_transitions;
        }
    };

    template<typename final_state_t>
    requires is_fa_final_state<final_state_t>
    class basic_nfa: public basic_fa<nfa_state, final_state_t> {
       protected:
        explicit basic_nfa()
        {
            this->start = this->add_state();
        }

        // 添加从state经过字符c向to的转换
        void add_transition(id_t state, char c, id_t to)
        {
            this->states.at(state).add_transition(c, to);
        }

        // 将charset中的字符添加为从start到final的转换
        void add_transition(const charset_t& chars, bool is_negated = false)
        {
            charset_t final_charset =
                is_negated ? (regex::ascii_printable_chars & ~chars) : chars;

            for (std::size_t i = 0; i < final_charset.size(); i++) {
                if (final_charset.test(i)) {
                    this->add_transition(this->get_start(), static_cast<char>(i),
                                         this->get_final());
                }
            }
        }

        // 添加从state到to的epsilon转换
        void add_epsilon_transition(id_t state, id_t to)
        {
            this->states.at(state).add_epsilon_transition(to);
        }

        // 内部辅助方法：将另一个NFA的状态和转换合并到当前NFA中
        // 返回偏移量，用于调整传入NFA的状态ID
        template<typename T>
        // todo: add constrines
        std::size_t merge_states(const T& other_nfa)
        {
            // 保存当前NFA的原始状态数量作为偏移量
            std::size_t offset = this->states.size();

            const auto& other_states = other_nfa.get_states();

            // 添加other_nfa的所有状态到当前NFA中
            for (const auto& s : other_states) {
                this->add_state();
            }

            // 复制other_nfa的转换，但需要调整ID偏移
            for (id_t i = 0; i < other_states.size(); i++) {
                const auto& other_state          = other_nfa.get_state(i);
                const auto& other_transition_map = other_state.get_transition_map();

                // 复制字符转换
                for (int input_idx = 0; input_idx < other_transition_map.size();
                     input_idx++) {
                    if (not other_transition_map[input_idx].empty()) {
                        char input_char = static_cast<char>(input_idx);

                        for (auto id : other_transition_map[input_idx]) {
                            this->add_transition(i + offset, input_char, id + offset);
                        }
                    }
                }

                // 处理epsilon转换
                for (auto id : other_state.get_epsilon_transition()) {
                    this->add_epsilon_transition(i + offset, id + offset);
                }
            }

            return offset;
        }

        // 连接当前NFA与传入的NFA
        // regexp: 当前NFA + next
        // 将传入的NFA连接到当前NFA之后
        void connect_with(const basic_nfa& next)
        {
            // 使用公共方法合并next的状态和转换
            std::size_t offset = this->merge_states(next);

            // 连接当前NFA的最终状态和next的起始状态
            this->add_epsilon_transition(this->get_final(), next.start + offset);

            // 更新当前NFA的最终状态为next的最终状态（加上偏移）
            this->set_final(next.get_final() + offset);
        }

        // 与传入的NFA进行选择操作
        // regexp: 当前NFA|other
        // 将传入的NFA与当前NFA进行选择操作（|）
        void select_with(const basic_nfa& other)
        {
            // 保存当前NFA的原始起始和最终状态
            id_t original_start = this->get_start();
            id_t original_final = this->get_final();

            // 使用公共方法合并other的状态和转换
            std::size_t offset = this->merge_states(other);

            // 创建新的起始状态和最终状态
            id_t new_start = this->add_state();
            id_t new_final = this->add_state();

            // 从新的起始状态到原来的两个NFA的起始状态添加epsilon转换
            this->add_epsilon_transition(new_start, original_start);
            this->add_epsilon_transition(new_start, other.start + offset);

            // 从原来的两个NFA的最终状态到新的最终状态添加epsilon转换
            this->add_epsilon_transition(original_final, new_final);
            this->add_epsilon_transition(other.get_final() + offset, new_final);

            // 更新当前NFA的起始和最终状态
            this->set_start(new_start);
            this->set_final(new_final);
        }

       public:
        friend struct std::formatter<basic_nfa>;

        class regex_error: public std::runtime_error {
           public:
            explicit regex_error(const std::string& msg): std::runtime_error(msg)
            {
            }
        };

        bool has_final(const closure_t& states) const
        {
            return states.contains(this->get_final());
        }

        // 合并两个NFA，实现选择操作（类似 | 操作符）
        basic_nfa operator|(const basic_nfa& other) const
        {
            basic_nfa result;

            // 保存当前NFA的原始起始和最终状态
            id_t original_start = this->get_start();
            id_t original_final = this->get_final();

            // 使用公共方法合并other的状态和转换
            std::size_t offset       = result.merge_states(*this);
            std::size_t other_offset = result.merge_states(other);

            // 创建新的起始状态和最终状态
            id_t new_start = result.add_state();
            id_t new_final = result.add_state();

            // 从新的起始状态到原来的两个NFA的起始状态添加epsilon转换
            result.add_epsilon_transition(new_start, original_start + offset);
            result.add_epsilon_transition(new_start, other.start + other_offset);

            // 从原来的两个NFA的最终状态到新的最终状态添加epsilon转换
            result.add_epsilon_transition(original_final + offset, new_final);
            result.add_epsilon_transition(other.get_final() + other_offset, new_final);

            // 更新当前NFA的起始和最终状态
            result.set_start(new_start);
            result.set_final(new_final);

            return result;
        }
    };
} // namespace regex
