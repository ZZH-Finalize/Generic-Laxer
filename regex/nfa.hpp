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
#include "final_state.hpp"

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

    class nfa: public basic_fa<__nfa_state, final_state> {
       public:
        using closure   = final_state::closure;

       protected:
        // 内部辅助方法：将另一个NFA的状态和转换合并到当前NFA中
        // 返回偏移量，用于调整传入NFA的状态ID
        std::size_t merge_nfa_states(const nfa& other_nfa);

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
            return states.contains(this->final);
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

// 为 nfa 类提供 std::format 支持
template<>
struct std::formatter<regex::nfa>
{
    std::string direction;

    constexpr auto parse(std::format_parse_context& ctx)
    {
        auto it = ctx.begin();

        while (*it != '}') {
            this->direction.push_back(*it++);
        }

        // 检查方向字符串是否有效
        if (this->direction != "LR" and this->direction != "TB"
            and this->direction != "RL" and not this->direction.empty()) {
            return ctx.end();
        }

        return it;
    }

    auto format(const regex::nfa& nfa, std::format_context& ctx) const
    {
        std::string result = "```mermaid\n";

        result += "stateDiagram-v2\n";

        if (not this->direction.empty()) {
            result += std::format("direction {}\n", this->direction);
        }

        result += "\n";

        auto& states = nfa.get_states();

        auto get_state_str = [&nfa](regex::nfa::state::id_t id) {
            if (id == nfa.get_start() || id == nfa.get_final()) {
                return std::string("[*]");
            } else {
                return std::format("state_{}", id);
            }
        };

        for (auto current_state_id = 0; current_state_id < states.size();
             current_state_id++) {
            if (current_state_id == nfa.get_final()) {
                continue;
            }

            const regex::nfa::state& state = states.at(current_state_id);
            std::string from               = get_state_str(current_state_id);

            // handle epsilon transitions
            for (auto& to_state : state.get_epsilon_transition()) {
                std::string to = get_state_str(to_state);

                result += std::format("{} --> {}\n", from, to);
            }

            // handle char transitions
            auto trans_map = state.get_transition_map();
            // 遍历所有可能的输入
            for (auto input = 0; input < trans_map.size(); input++) {
                auto& trans = trans_map.at(input);

                // 对输入字符, 遍历可能的目标状态
                for (auto& to_state : trans) {
                    std::string to = get_state_str(to_state);

                    result += std::format("{} --> {} : {}\n", from, to,
                                          static_cast<char>(input));
                }
            }
        }

        result += "```";

        return std::format_to(ctx.out(), "{}", result);
    }
};
