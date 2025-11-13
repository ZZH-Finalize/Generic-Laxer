#pragma once

#include <any>
#include <bitset>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iterator>
#include <exception>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include <set>

#include "basic_fa.hpp"
#include "basic_state.hpp"

namespace regex {

    class __nfa_state: public basic_state<std::vector<std::uint32_t>> {
       private:
        transition_map_item_t epsilon_transitions;

       public:
        inline void add_epsilon_transition(id_t to)
        {
            this->epsilon_transitions.push_back(to);
        }

        inline const auto& get_epsilon_transition(void) const
        {
            return this->epsilon_transitions;
        }
    };

    class nfa: public basic_fa<__nfa_state> {
       protected:
        id_t final;

        explicit nfa()
        {
            this->start = this->add_state();
            this->final = this->add_state();
        }

        void set_final(state::id_t id) noexcept
        {
            this->final = id;
        }

        const auto& get_final(void) const noexcept
        {
            return this->final;
        }

        // 添加从state经过字符c向to的转换
        void add_transition(state::id_t state, char c, state::id_t to)
        {
            this->states.at(state).add_transition(c, to);
        }

        // 将charset中的字符添加为从start到final的转换
        void add_transition(const charset_t& chars, bool is_negated = false)
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

        // 添加从state到to的epsilon转换
        void add_epsilon_transition(state::id_t state, state::id_t to)
        {
            this->states.at(state).add_epsilon_transition(to);
        }

        // 内部辅助方法：将另一个NFA的状态和转换合并到当前NFA中
        // 返回偏移量，用于调整传入NFA的状态ID
        std::size_t merge_states(const nfa& other_nfa);

        // 连接当前NFA与传入的NFA
        // regexp: 当前NFA + next
        // 将传入的NFA连接到当前NFA之后
        void connect_with(const nfa& next);

        // 与传入的NFA进行选择操作
        // regexp: 当前NFA|other
        // 将传入的NFA与当前NFA进行选择操作（|）
        void select_with(const nfa& other);

       public:
        friend class builder;
        friend struct std::formatter<nfa>;

        class regex_error: public std::runtime_error {
           public:
            explicit regex_error(const std::string& msg): std::runtime_error(msg)
            {
            }
        };

        bool has_final(const closure& states) const
        {
            return states.contains(this->get_final());
        }

        // 合并两个NFA，实现选择操作（类似 | 操作符）
        nfa operator|(const nfa& other) const;
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
    concept has_nfa_op = requires(const T& t, nfa::state::id_t state,
                                  const nfa::closure& closure) {
        { t.get_start() } -> std::same_as<nfa::state::id_t>;
        { t.has_final(closure) } -> std::same_as<bool>;
        { t.get_state(state) } -> has_nfa_state_op;
        { t.get_states() } -> std::ranges::range;

        requires has_nfa_state_op<std::ranges::range_value_t<decltype(t.get_states())>>;
    };

    // 是否为NFA抽象类型(regex::nfa及其其子类,
    // 或者与regex::nfa行为相同的任何类都可以算作NFA)
    template<typename T>
    concept is_nfa = std::is_same_v<T, nfa> or is_base_of_nfa_v<T> or has_nfa_op<T>;

    // 是否具有获取元数据的能力
    template<typename NFA>
    concept has_metadata = requires(const NFA& nfa, const nfa::closure& closure) {
        { nfa.get_metadata(closure) } -> std::same_as<std::any>;
    };
} // namespace regex
