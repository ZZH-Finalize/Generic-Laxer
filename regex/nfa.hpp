#pragma once

#include <algorithm>
#include <concepts>
#include <format>
#include <optional>
#include <type_traits>

#include "basic_nfa.hpp"
#include "basic_dfa.hpp"
#include "regex_typedef.hpp"

namespace regex {

    // NFA与NFA的终态类型基类, 实现了最基础的concepts要求
    class final_state_t {
       private:
        id_t state_id;

       public:
        final_state_t(id_t id = 0): state_id(id)
        {
        }

        operator id_t(void) const noexcept
        {
            return this->state_id;
        }

        void set_state_id(id_t id) noexcept
        {
            this->state_id = id;
        }
    };

    // 定义regex::nfa
    class nfa: public basic_nfa<final_state_t> {
       public:
        friend class builder;

        // 添加从基类basic_nfa构造nfa的构造函数
        nfa(const basic_nfa& base_nfa)
        {
            // 复制基类的状态和结构
            this->states = base_nfa.get_states();

            // 设置起始和结束状态
            this->set_start(base_nfa.get_start());
            this->set_final(base_nfa.get_final());
        }

        nfa(basic_nfa&& base_nfa)
        {
            // 拿走基类的状态和结构
            this->states = std::move(base_nfa.get_states());

            // 设置起始和结束状态
            this->set_start(base_nfa.get_start());
            this->set_final(base_nfa.get_final());
        }

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
        : formatter<regex::basic_nfa<regex::nfa::final_state_t>, CharT>
    {
        using formatter<regex::basic_nfa<regex::nfa::final_state_t>, CharT>::parse;
        using formatter<regex::basic_nfa<regex::nfa::final_state_t>, CharT>::format;
    };

} // namespace std
