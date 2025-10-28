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
            // 使用特殊值表示epsilon转换，避免与实际字符冲突
            // 使用255作为epsilon转换的标记，这是一个不太可能在正则表达式中使用的字符
            this->transitions[static_cast<char>(255)].push_back(
                to); // 使用255表示epsilon转换
        }

        // 获取转换映射的常量引用，用于复制NFA结构
        const transition_map& get_transitions() const
        {
            return this->transitions;
        }
    };

} // namespace regex
