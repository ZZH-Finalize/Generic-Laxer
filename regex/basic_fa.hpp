#pragma once

#include <algorithm>
#include <concepts>
#include <type_traits>
#include <utility>
#include <vector>
#include <set>
#include <bitset>

#include "basic_state.hpp"
#include "regex_concepts.hpp"
#include "regex_typedef.hpp"

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

    // 检查是否有add_epsilon_transition函数
    template<typename T>
    concept has_add_epsilon_transition =
        requires(T &t, id_t id) { t.add_epsilon_transition(id); };

    // 约束规则如下
    // 1. state_t必须是basic_state的子类
    template<typename state_t, typename _final_state_t>
    requires is_fa_state<state_t> and is_fa_final_state<_final_state_t>
    class basic_fa {
       public:
        // 添加state别名
        using state                 = state_t;
        using id_t                  = state_t::id_t;
        using closure_t             = closure_t;
        using transition_map_item_t = state::transition_map_item_t;
        using final_state_t         = _final_state_t;

        // 非法id值
        inline static const id_t invalid_state = std::numeric_limits<id_t>::max();

       protected:
        std::vector<state> states;
        id_t start;
        final_state_t final;

        explicit basic_fa(void): start {}, final {}
        {
        }

        template<typename... Args>
        inline id_t add_state(Args &&...args)
        {
            this->states.emplace_back(std::forward<Args>(args)...);

            return this->states.size() - 1;
        }

        inline void set_start(id_t start) noexcept
        {
            this->start = start;
        }

        inline void set_final(const final_state_t &final) noexcept
        {
            this->final = final;
        }

        inline void set_final(final_state_t &&final) noexcept
        {
            this->final = std::move(final);
        }

        template<typename... Args>
        inline void add_final(Args &&...args)
        requires has_emplace_back<final_state_t>
        {
            this->final.emplace_back(std::forward<Args>(args)...);
        }

        template<typename... Args>
        inline void add_final(Args &&...args)
        requires has_insert<final_state_t>
        {
            this->final.insert(std::forward<Args>(args)...);
        }

        // 为nfa和dfa及其子类实现, 拷贝赋值版本
        inline void set_transition(id_t state, char c, const transition_map_item_t &to)
        {
            this->states.at(state).set_transition(c, to);
        }

        // 为nfa和dfa及其子类实现, 移动赋值版本
        inline void set_transition(id_t state, char c, transition_map_item_t &&to)
        {
            this->states.at(state).set_transition(c, std::move(to));
        }

       public:
        inline auto get_start(void) const noexcept
        {
            return this->start;
        }

        inline const auto &get_final(void) const noexcept
        {
            return this->final;
        }

        inline const auto &get_state(id_t state) const noexcept
        {
            return this->states.at(state);
        }

        inline const auto &get_states(void) const noexcept
        {
            return this->states;
        }

        // 获取状态数量
        inline std::size_t get_state_count(void) const noexcept
        {
            return this->states.size();
        }
    };

} // namespace regex
