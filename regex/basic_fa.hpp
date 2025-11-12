#pragma once

#include <algorithm>
#include <concepts>
#include <type_traits>
#include <vector>
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
    concept is_fa_state = std::is_base_of_v<bsic_state_base, T>;

    // 检查两个类是否定义了相同的id_t
    template<typename T, typename U>
    concept is_id_same = std::is_same_v<typename T::id_t, typename U::id_t>;

    // 检查是否有add_epsilon_transition函数
    template<typename T>
    concept has_add_epsilon_transition =
        requires(T &t, T::id_t id) { t.add_epsilon_transition(id); };

    // 约束规则如下
    // 1. state_t必须是basic_state的子类
    // 2. final_state_t必须与state_t定义相同的id_t类型
    // 3. final_state_t必须能够转换为state_t::id_t类型
    template<typename state_t, typename final_state_t>
    requires is_fa_state<state_t> and is_id_same<state_t, final_state_t>
             and std::convertible_to<final_state_t, typename state_t::id_t>
    class basic_fa {
       public:
        // 添加state别名
        using state                 = state_t;
        using transition_map_item_t = state::transition_map_item_t;
        using charset_t             = std::bitset<256>;

        // 非法id值
        inline static const state::id_t invalid_state =
            std::numeric_limits<typename state::id_t>::max();

       protected:
        std::vector<state> states;
        state::id_t start;
        final_state_t final;

        explicit basic_fa(void): start(this->add_state()), final(this->add_state())
        {
        }

        state::id_t add_state(void)
        {
            this->states.emplace_back();
            return this->states.size() - 1;
        }

        inline void set_start(state::id_t start)
        {
            this->start = start;
        }

        // 拷贝赋值版本
        inline void set_final(const final_state_t &final)
        {
            this->final = final;
        }

        // 移动赋值版本
        // void set_final(final_state_t &&final)
        // requires(not std::is_trivial_v<final_state_t>)
        // {
        //     this->final = std::move(final);
        // }

        // 为nfa及其子类实现, 添加从state经过字符c向to的转换
        void add_transition(state::id_t state, char c, state::id_t to)
        requires is_container<transition_map_item_t>
        {
            this->states.at(state).add_transition(c, to);
        }

        // 为nfa及其子类实现, 将charset中的字符添加为从start到final的转换
        void add_transition(const charset_t &chars, bool is_negated = false)
        {
            charset_t final_charset =
                is_negated ? (regex::ascii_printable_chars & ~chars) : chars;

            for (std::size_t i = 0; i < final_charset.size(); i++) {
                if (final_charset.test(i)) {
                    this->add_transition(this->get_start(), static_cast<char>(i),
                                         this->get_final());
                }
            }
        }

        // 为nfa及其子类实现, 添加从state到to的epsilon转换
        void add_epsilon_transition(state::id_t state, state::id_t to)
        requires has_add_epsilon_transition<state_t>
        {
            this->states.at(state).add_epsilon_transition(to);
        }

        // 为nfa和dfa及其子类实现, 拷贝赋值版本
        void set_transition(state::id_t state, char c, const transition_map_item_t &to)
        {
            this->states.at(state).set_transition(c, to);
        }

        // 为nfa和dfa及其子类实现, 移动赋值版本
        void set_transition(state::id_t state, char c, transition_map_item_t &&to)
        {
            this->states.at(state).set_transition(c, std::move(to));
        }

       public:
        inline auto get_start(void) const noexcept
        {
            return this->start;
        }

        inline auto get_final(void) const noexcept
        {
            return this->final;
        }

        // todo: forward call to final_state_t
        // bool has_final(const closure &states) const
        // {
        //     return states.contains(this->final);
        // }

        inline const auto &get_state(state::id_t state) const noexcept
        {
            return this->states.at(state);
        }

        inline const auto &get_states(void) const noexcept
        {
            return this->states;
        }
    };

} // namespace regex
