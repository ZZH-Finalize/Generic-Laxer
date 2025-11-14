#pragma once

#include <concepts>
#include <format>
#include <optional>
#include <type_traits>

#include "basic_nfa.hpp"
#include "basic_dfa.hpp"

namespace regex {

    // 定义regex::nfa
    class nfa: public basic_nfa<id_t> {
       public:
        friend class builder;

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

    // 是否具有与regex::nfa相同的操作(以子集构造算法所需要操作的来看)
    template<typename T>
    concept has_nfa_op = requires(const T& t, id_t state, const closure_t& closure) {
        // T必须提供dfa来指明对应的DFA类型
        typename T::dfa;
        // T必须提供自身使用的终态类型(如果终态是容器, 则代表容器内部类型)
        typename T::final_state_id_t;

        // 需要获取T的起始id
        { t.get_start() } -> std::same_as<id_t>;
        // 需要T给出闭包中的终态
        {
            t.find_final(closure)
        } -> std::same_as<std::optional<typename T::final_state_id_t>>;
        // 需要从T中获取具体的某个state对象, 返回的对象需要实现与nfa::state相同的公共函数
        { t.get_state(state) } -> has_nfa_state_op;
        // 需要T给出所有的state, 且需要以容器形式提供
        { t.get_states() } -> std::ranges::range;

        // 获取到的容器内部对象需要实现与nfa::state相同的公共函数
        requires has_nfa_state_op<std::ranges::range_value_t<decltype(t.get_states())>>;
    };

    // 是否为NFA抽象类型(regex::nfa及其子类, 或者与其行为相同的任何类都可以算作NFA)
    template<typename T>
    concept is_nfa = std::is_same_v<T, nfa> or is_base_of_nfa_v<T> or has_nfa_op<T>;

    // 是否为DFA类型, 由于DFA均为自动生成的类型, 只需要根据DFA公共基类判断是否为派生类即可
    template<typename T>
    concept is_dfa = std::is_base_of_v<basic_dfa_base, T>;
} // namespace regex

namespace std {
    template<typename CharT>
    struct formatter<regex::nfa, CharT>
        : formatter<regex::basic_nfa<regex::nfa::id_t>, CharT>
    {
        using formatter<regex::basic_nfa<regex::nfa::id_t>, CharT>::parse;
        using formatter<regex::basic_nfa<regex::nfa::id_t>, CharT>::format;
    };

} // namespace std
