#pragma once

#include <vector>
#include <set>
#include "regex/nfa.hpp"
#include "regex/final_state.hpp"

namespace laxer {
    class nfa: protected regex::nfa {
       public:
        using accept_states_t = std::set<regex::final_state>;

       private:
        // 所有NFA的终态以及规则id
        accept_states_t accept_states;

        // 规则id计数
        state::id_t current_rule_id;

       public:
        explicit nfa(state::id_t rule_id_base = 0)
            : regex::nfa(), current_rule_id(rule_id_base)
        {
        }

        // 添加单个regex::nfa到当前NFA
        void add_nfa(const regex::nfa& input_nfa)
        {
            auto offset = this->merge_nfa_states(input_nfa);

            // 从起始状态创建epsilon转换到新添加NFA的起始状态
            this->add_epsilon_transition(this->get_start(),
                                         input_nfa.get_start() + offset);

            // 把input_nfa的终态添加到accept_states中, 并分配规则id
            this->accept_states.emplace(input_nfa.get_final() + offset,
                                        this->current_rule_id++);
        }

        void add_nfa(const std::vector<regex::nfa>& input_nfas)
        {
            for (const auto& input_nfa : input_nfas) {
                this->add_nfa(input_nfa);
            }
        }

        bool has_final(const closure& states) const
        {
            for (auto state : states) {
                if (this->accept_states.contains(state)) {
                    return true;
                }
            }

            return false;
        }
    };
} // namespace laxer
