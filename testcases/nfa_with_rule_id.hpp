#pragma once

#include <optional>
#include <ranges>
#include <set>

#include "regex/nfa.hpp"
#include "regex/basic_nfa.hpp"
#include "regex_typedef.hpp"

namespace laxer_test {
    class final_state_t {
       public:
        using id_t = regex::id_t;

       private:
        id_t state_id, rule_id;

       public:
        final_state_t(id_t state_id = 0, id_t rule_id = 0)
            : state_id(state_id), rule_id(rule_id)
        {
        }

        operator id_t(void) const noexcept
        {
            return this->state_id;
        }

        id_t get_rule_id(void) const noexcept
        {
            return this->rule_id;
        }

        void copy_metadata(const final_state_t& other)
        {
            this->rule_id = other.rule_id;
        }
    };

    class nfa: public regex::basic_nfa<std::set<final_state_t>> {
       private:
        id_t current_rule_id;

       public:
        using final_state_id_t = typename basic_nfa::final_state_id_t;

        nfa(id_t initial_rule_id = 0): current_rule_id(initial_rule_id)
        {
        }

        // 添加单个regex::nfa到当前NFA
        void add_nfa(const regex::nfa& input_nfa)
        {
            auto offset = this->merge_states(input_nfa);

            // 从起始状态创建epsilon转换到新添加NFA的起始状态
            this->add_epsilon_transition(this->get_start(),
                                         input_nfa.get_start() + offset);

            // 把input_nfa的终态添加到accept_states中, 并分配规则id
            this->add_final(input_nfa.get_final() + offset, this->current_rule_id++);
        }

        template<typename T>
        requires std::ranges::range<T>
        void add_nfa(const T& input_nfas)
        {
            for (const auto& input_nfa : input_nfas) {
                this->add_nfa(input_nfa);
            }
        }

        std::optional<final_state_id_t> find_final(const closure_t& states) const
        {
            for (const auto& state : states) {
                const auto& final = this->get_final().find(state);
                if (final != this->get_final().end()) {
                    return *final;
                }
            }

            return std::nullopt;
        }
    };

} // namespace laxer_test
