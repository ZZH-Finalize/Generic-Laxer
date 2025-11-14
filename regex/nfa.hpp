#pragma once

#include <concepts>
#include <type_traits>

#include "basic_nfa.hpp"

#include "dfa.hpp"

namespace regex {

    // 定义regex::nfa
    class nfa: public basic_nfa<id_t> {
       public:
        friend class builder;
        using to_type = regex::dfa;

       protected:
        nfa()
        {
            this->final = this->add_state();
        }
    };

    // nfa相关约束
    // 是否具有与regex::nfa::state相同的操作
    template<typename T>
    concept has_nfa_state_op = requires(const T& t, char input) {
        // 约束T类型必须有get_transition_map方法
        { t.get_transition_map() } -> std::same_as<const nfa::state::transition_map_t&>;

        // 约束T类型必须有get_transition方法
        {
            t.get_transition(input)
        } -> std::same_as<const nfa::state::transition_map_item_t&>;

        // 约束T类型必须有get_epsilon_transition方法
        {
            t.get_epsilon_transition()
        } -> std::same_as<const nfa::state::transition_map_item_t&>;
    };

    // 是否为regex::nfa派生类
    template<typename T>
    concept is_base_of_nfa_v = std::is_base_of_v<nfa, T>;

    // 是否具有与regex::nfa相同的操作(以子集构造算法所需要的来看)
    template<typename T>
    concept has_nfa_op = requires(const T& t, id_t state, const closure_t& closure) {
        typename T::to_type;
        { t.get_start() } -> std::same_as<id_t>;
        { t.has_final(closure) } -> std::same_as<bool>;
        { t.get_state(state) } -> has_nfa_state_op;
        { t.get_states() } -> std::ranges::range;

        requires has_nfa_state_op<std::ranges::range_value_t<decltype(t.get_states())>>;
    };

    // 是否为NFA抽象类型(regex::nfa及其其子类,
    // 或者与regex::nfa行为相同的任何类都可以算作NFA)
    template<typename T>
    concept is_nfa = std::is_same_v<T, nfa> or is_base_of_nfa_v<T> or has_nfa_op<T>;
} // namespace regex
