#pragma once

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <array>
#include <vector>

template<typename T>
concept id_type_c = std::is_integral_v<T>;

namespace regex {

    inline constexpr std::bitset<256> ascii_printable_chars = [] {
        std::bitset<256> bs;
        for (int i = ' '; i <= '~'; ++i) {
            bs.set(i);
        }
        return bs;
    }();

    class state {
       public:
        using id_type          = std::uint32_t;
        using transition_t     = std::vector<id_type>;
        using transition_map_t = std::array<transition_t, 257>;

        static const id_type epsilon_id = 256;

       private:
        transition_map_t transitions;

       public:
        inline void add_transition(char input, id_type to)
        {
            this->transitions[input].push_back(to);
        }

        inline void add_epsilon_transition(id_type to)
        {
            this->transitions[this->epsilon_id].push_back(to);
        }

        // 获取转换映射的常量引用，用于复制NFA结构
        const transition_map_t& get_transition_map(void) const noexcept
        {
            return this->transitions;
        }

        const transition_t& get_transition(id_type id) const
        {
            return this->transitions[id];
        }

        const transition_t& get_epsilon_transition(void) const
        {
            return this->get_transition(this->epsilon_id);
        }
    };

} // namespace regex
