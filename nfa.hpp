#pragma once

#include <vector>
#include <set>
#include "regex/nfa.hpp"
#include "accept_state.hpp"

namespace laxer {
    class nfa {
       public:
        using state_id_t      = regex::nfa::state::id_t;
        using rule_id_t       = accept_state_t::id_t;
        using states_t        = regex::nfa::states_t;
        using accept_states_t = std::set<accept_state_t>;

       private:
        // 起始状态ID
        state_id_t start;

        // 所有NFA合并后的状态
        states_t states;

        // 所有NFA的终态以及规则id
        accept_states_t accept_states;

        // 规则id计数
        rule_id_t current_rule_id;

        state_id_t add_state(void)
        {
            this->states.emplace_back();

            return this->states.size() - 1;
        }

        // 获取特定状态的引用（用于修改状态转换）
        regex::nfa::state& get_state(state_id_t id)
        {
            return this->states[id];
        }

        void set_start(state_id_t start)
        {
            this->start = start;
        }

        rule_id_t alloc_rule_id(void)
        {
            return this->current_rule_id++;
        }

        // 添加接受状态
        void emplace_accept_state(state_id_t id)
        {
            this->accept_states.emplace(id, this->alloc_rule_id());
        }

        // 添加从start到另一个nfa起始状态的epsilon转换
        void add_start_transition(state_id_t to)
        {
            this->states[this->get_start()].add_epsilon_transition(to);
        }

       public:
        explicit nfa(void): current_rule_id(0)
        {
            this->set_start(this->add_state());
        }

        //
        void add_nfa(const regex::nfa& input_nfa);

        void add_nfa(const std::vector<regex::nfa>& input_nfas)
        {
            for (const auto& input_nfa : input_nfas) {
                this->add_nfa(input_nfa);
            }
        }

        // 获取起始状态
        regex::nfa::state::id_t get_start(void) const
        {
            return this->start;
        }

        // 获取所有状态
        const states_t& get_states(void) const
        {
            return this->states;
        }

        // 获取接受状态
        const accept_states_t& get_accept_states(void) const
        {
            return this->accept_states;
        }

        // 获取特定状态的常量引用
        const regex::nfa::state& get_state(state_id_t id) const
        {
            return this->states[id];
        }
    };
} // namespace laxer
