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
    // 是否为NFA抽象类型(regex::nfa及其子类, 或者与其行为相同的任何类都可以算作NFA)
    template<typename T>
    concept is_nfa = has_nfa_op<T>;

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
