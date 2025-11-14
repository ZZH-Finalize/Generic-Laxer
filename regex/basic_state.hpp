#pragma once

#include <array>
#include <limits>

#include "regex_concepts.hpp"
#include "regex_typedef.hpp"

namespace regex {
    class basic_state_base {};

    template<typename T>
    class basic_state: private basic_state_base {
       public:
        using id_t                  = regex::id_t;
        using transition_map_item_t = T;
        using closure_t             = regex::closure_t;
        using transition_map_t =
            std::array<transition_map_item_t,
                       std::numeric_limits<unsigned char>::max() + 1>;

        inline void set_transition(char ch, const T& id)
        {
            this->transition_map[static_cast<unsigned char>(ch)] = id;
        }

        inline void set_transition(char ch, T&& id)
        {
            this->transition_map[static_cast<unsigned char>(ch)] = std::move(id);
        }

        inline void add_transition(char ch, id_t to)
        requires has_push_back<T>
        {
            this->transition_map[static_cast<unsigned char>(ch)].push_back(to);
        }

        inline void add_transition(char ch, id_t to)
        requires has_insert<T>
        {
            this->transition_map[static_cast<unsigned char>(ch)].insert(to);
        }

        inline const auto& get_transition(char ch) const
        {
            return this->transition_map[static_cast<unsigned char>(ch)];
        }

        inline const auto& get_transition_map(void) const noexcept
        {
            return this->transition_map;
        }

       protected:
        transition_map_t transition_map;
    };

} // namespace regex
