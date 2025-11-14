#pragma once

#include "nfa.hpp"
#include "builder.hpp"
#include "formatters.hpp"

namespace regex {

    inline auto build_nfa(std::string_view exp)
    {
        return builder::build(exp);
    }

    template<typename NFA>
    requires is_nfa<NFA>
    inline auto to_dfa(const NFA& nfa)
    {
        return builder::build(nfa);
    }

    static inline auto build_dfa(std::string_view exp)
    {
        return to_dfa(build_nfa(exp));
    }

    template<typename NFA>
    requires is_nfa<NFA>
    static inline auto build(const NFA& nfa)
    {
        return to_dfa(nfa);
    }

    static inline auto build(std::string_view exp)
    {
        return build_dfa(exp);
    }

    template<typename DFA>
    requires is_dfa<DFA>
    inline auto minimize(const DFA& dfa)
    {
        // return builder::minimize(dfa);
        return dfa;
    }

    // 重载操作符，支持函数名、函数指针、函数对象等
    template<typename T>
    requires is_nfa<T>
    inline auto operator|(const T& input, typename T::dfa (*f)(const T&))
    {
        return f(input);
    }

    // 通用版本，支持函数对象和lambda
    template<typename T, typename F>
    requires is_nfa<T> and requires(const T& input, F&& f) {
        { f(input) } -> std::same_as<typename T::dfa>;
    }
    inline auto operator|(const T& input, F&& f)
    {
        return std::forward<F>(f)(input);
    }

    // 重载操作符，支持函数名、函数指针、函数对象等
    template<typename T>
    requires is_dfa<T>
    inline auto operator|(const T& input, T (*f)(const T&))
    {
        return f(input);
    }

    // 通用版本，支持函数对象和lambda
    template<typename T, typename F>
    requires is_dfa<T> and requires(const T& input, F&& f) {
        { f(input) } -> std::same_as<T>;
    }
    inline auto operator|(const T& input, F&& f)
    {
        return std::forward<F>(f)(input);
    }

} // namespace regex
