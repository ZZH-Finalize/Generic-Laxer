#pragma once

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <unordered_map>
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
        using id_type        = std::uint32_t;
        using transition_map = std::unordered_map<char, std::vector<id_type>>;

       private:
        inline static state::id_type id_counter = 1;
        id_type __id;
        transition_map transitions;

       public:
        state(): __id(this->id_counter++)
        {
        }

        void add_transition(char input, id_type to)
        {
            this->transitions[input].push_back(to);
        }

        void add_epsilon_transition(id_type to)
        {
            this->transitions['\0'].push_back(to); // 使用空字符表示epsilon转换
        }

        // 获取转换映射的常量引用，用于复制NFA结构
        const transition_map& get_transitions() const
        {
            return this->transitions;
        }
    };

} // namespace regex
