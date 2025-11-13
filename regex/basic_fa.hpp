#pragma once

#include <algorithm>
#include <concepts>
#include <type_traits>
#include <utility>
#include <vector>
#include <set>
#include <bitset>

#include "basic_state.hpp"

namespace regex {
    inline constexpr std::bitset<256> ascii_printable_chars = [] {
        std::bitset<256> bs;
        for (int i = ' '; i <= '~'; ++i) {
            bs.set(i);
        }
        return bs;
    }();

    // 检查是否为basic_state的子类
    // template<typename T>
    // concept is_fa_state = requires(void) {
    //     []<typename U>(basic_state<U> *) {}(static_cast<T *>(nullptr));
    // };
    template<typename T>
    concept is_fa_state = std::is_base_of_v<basic_state_base, T>;

    // 检查两个类是否定义了相同的id_t
    template<typename T, typename U>
    concept is_id_same = std::is_same_v<typename T::id_t, typename U::id_t>;

    // 检查是否有add_epsilon_transition函数
    template<typename T>
    concept has_add_epsilon_transition =
        requires(T &t, T::id_t id) { t.add_epsilon_transition(id); };

    // 约束规则如下
    // 1. state_t必须是basic_state的子类
    template<typename state_t>
    requires is_fa_state<state_t>
    class basic_fa {
       public:
        // 添加state别名
        using state                 = state_t;
        using id_t                  = state_t::id_t;
        using transition_map_item_t = state::transition_map_item_t;
        using charset_t             = std::bitset<256>;

        // 非法id值
        inline static const state::id_t invalid_state =
            std::numeric_limits<typename state::id_t>::max();

       protected:
        std::vector<state> states;
        state::id_t start;

        explicit basic_fa(void): start(0)
        {
        }

        template<typename... Args>
        inline state::id_t add_state(Args &&...args)
        {
            this->states.emplace_back(std::forward<Args>(args)...);

            return this->states.size() - 1;
        }

        inline void set_start(state::id_t start)
        {
            this->start = start;
        }

        // 为nfa和dfa及其子类实现, 拷贝赋值版本
        inline void set_transition(state::id_t state, char c, const transition_map_item_t &to)
        {
            this->states.at(state).set_transition(c, to);
        }

        // 为nfa和dfa及其子类实现, 移动赋值版本
        inline void set_transition(state::id_t state, char c, transition_map_item_t &&to)
        {
            this->states.at(state).set_transition(c, std::move(to));
        }

       public:
        inline auto get_start(void) const noexcept
        {
            return this->start;
        }

        inline const auto &get_state(state::id_t state) const noexcept
        {
            return this->states.at(state);
        }

        inline const auto &get_states(void) const noexcept
        {
            return this->states;
        }

        // 获取状态数量
        inline std::size_t get_state_count(void) const
        {
            return this->states.size();
        }
    };

} // namespace regex
