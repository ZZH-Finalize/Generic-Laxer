#pragma once

#include "nfa.hpp"
#include "dfa.hpp"
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

    inline auto minimize(const dfa& dfa)
    {
        return builder::minimize(dfa);
    }

    template<typename T>
    concept regexp_res = is_nfa<T> || std::is_same_v<T, dfa>;

    // 为支持函数指针和函数对象，扩展概念
    template<typename F, typename T>
    concept regexp_op = regexp_res<T> && requires(F&& f, const T& input) {
        { f(input) } -> std::same_as<dfa>;
    };

    // 重载操作符，支持函数名、函数指针、函数对象等
    template<typename T>
    requires regexp_res<T>
    inline auto operator|(const T& input, dfa (*f)(const T&))
    {
        return f(input);
    }

    // 通用版本，支持函数对象和lambda
    template<typename T, typename F>
    requires regexp_res<T> && regexp_op<F, T>
    inline auto operator|(const T& input, F&& f)
    {
        return std::forward<F>(f)(input);
    }

} // namespace regex
