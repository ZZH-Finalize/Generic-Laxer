#pragma once

#include <format>
#include <set>
#include <string>

#include "regex/nfa.hpp"
#include "token.hpp"

namespace laxer {
    class final_state_t {
       public:
        using id_t = regex::id_t;

       private:
        id_t state_id;
        std::string rule_name;
        // std::function<> action;

       public:
    };

    class nfa: public regex::basic_nfa<std::set<token>> {
       private:
        id_t current_token_id;

       public:
        using final_state_id_t = typename basic_nfa::final_state_id_t;

        nfa(id_t initial_token_id = 0): current_token_id(initial_token_id)
        {
        }

        // 添加单个regex::nfa到当前NFA
        void add_nfa(const regex::nfa& input_nfa, const token::action_t& cb = {},
                     std::string name = {})
        {
            auto offset = this->merge_states(input_nfa);

            // 从起始状态创建epsilon转换到新添加NFA的起始状态
            this->add_epsilon_transition(this->get_start(),
                                         input_nfa.get_start() + offset);

            // 自动生成规则名
            if (name.empty()) {
                name = std::format("rule_{}", this->current_token_id + 1);
            }

            // 把input_nfa的终态添加到accept_states中, 并填充元数据
            this->add_final(input_nfa.get_final() + offset, this->current_token_id++, cb,
                            name);
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

} // namespace laxer
