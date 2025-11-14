#pragma once

#include "regex/basic_dfa.hpp"

namespace laxer {
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
    };

    class dfa: public regex::basic_dfa<std::set<final_state_t>> {
       public:
        dfa()
        {
        }
    };

} // namespace laxer
