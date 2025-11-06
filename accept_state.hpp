#pragma once

#include "regex/nfa.hpp"

namespace laxer {

    class accept_state_t {
       public:
        using id_t = regex::nfa::state::id_t;

       private:
        id_t state_id, rule_id;

       public:
        explicit accept_state_t(id_t state_id, id_t rule_id)
            : state_id(state_id), rule_id(rule_id)
        {
        }

        // 获取状态ID
        id_t get_state_id(void) const
        {
            return this->state_id;
        }

        // 获取规则ID
        id_t get_rule_id(void) const
        {
            return this->rule_id;
        }

        // 为std::set实现比较操作符
        bool operator<(const accept_state_t& other) const
        {
            return this->state_id < other.state_id;
        }
    };

} // namespace laxer
