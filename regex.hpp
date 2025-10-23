#pragma once

#include <cstdint>
#include <map>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

template<typename T>
concept id_type_c = std::is_integral_v<T>;

namespace regex {

    template<id_type_c id_type>
    class state {
       public:
        using transition_map = std::unordered_map<char, std::vector<id_type>>;

       private:
        static int id_counter;
        id_type __id;
        transition_map transitions;

       public:
        state(): __id(this->id_counter++)
        {
        }

        void add_transition(char input, id_type to_state)
        {
            this->transitions[input].push_back(to_state);
        }

        void add_epsilon_transition(id_type to_state)
        {
            this->transitions['\0'].push_back(to_state); // 使用空字符表示epsilon转换
        }
    };

    template<id_type_c id_type>
    class nfa {
       private:
        std::array<state<id_type>, 128> states;
        id_type start, final;

        static nfa parse_char(std::string_view str)
        {
            
        }

       public:
        nfa(): start(0), final(0)
        {
        }

        id_type add_state(void)
        {
            this->states.emplace_back();
            return this->states.size() - 1;
        }

        void set_start(id_type start)
        {
            this->start = start;
        }

        void set_final(id_type final)
        {
            this->final = final;
        }
    };

    class dfa {};

} // namespace regex
