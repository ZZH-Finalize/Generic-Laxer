#pragma once

#include <map>

#include "basic_nfa.hpp"
#include "nfa.hpp"

namespace std {
    // 为 nfa 类提供 std::format 支持
    template<typename T, typename CharT>
    requires regex::is_fa_final_state<T>
    struct formatter<regex::basic_nfa<T>, CharT>
    {
        using nfa = regex::basic_nfa<T>;
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

        template<typename FormatContext>
        auto format(const nfa& nfa, FormatContext& ctx) const
        {
            std::string result = "```mermaid\n";

            result += "stateDiagram-v2\n";

            if (not this->direction.empty()) {
                result += std::format("direction {}\n", this->direction);
            }

            result += "\n";

            auto& states = nfa.get_states();

            auto get_state_str = [&nfa](regex::id_t id) {
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

                const typename nfa::state& state = states.at(current_state_id);
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

    // 为 dfa 类提供 std::format 支持
    template<typename T, typename CharT>
    requires regex::is_fa_final_state_t<T>
    struct formatter<regex::basic_dfa<T>, CharT>
    {
        using dfa = regex::basic_dfa<T>;

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

        template<typename FormatContext>
        auto format(const dfa& dfa, FormatContext& ctx) const
        {
            std::string result = "```mermaid\n";

            result += "stateDiagram-v2\n";

            if (not this->direction.empty()) {
                result += std::format("direction {}\n", this->direction);
            }

            result += "\n";

            auto get_state_str = [&dfa](regex::id_t id) {
                auto& final_states = dfa.get_final();
                std::string state_str;

                if (id == dfa.get_start()) {
                    // return std::string("[*]");
                    state_str += "start_";
                }

                if (final_states.contains(id)) {
                    // return std::string("[*]");
                    state_str += "final_";
                }

                if (not state_str.empty()) {
                    return std::format("{}{}", state_str, id);
                }

                return std::format("state_{}", id);
            };

            auto& states = dfa.get_states();

            std::map<std::size_t, std::set<char>> merge_map;

            for (regex::id_t current_state_id = 0; current_state_id < states.size();
                 current_state_id++) {
                auto& state     = states[current_state_id];
                auto& trans_map = state.get_transition_map();

                for (auto input_char = 0; input_char < trans_map.size(); input_char++) {
                    auto to_state_id = trans_map[input_char];
                    if (to_state_id == dfa::invalid_state) {
                        continue;
                    } else if (current_state_id == to_state_id) {
                        merge_map[current_state_id].insert(input_char);
                        continue;
                    }

                    const auto& from = get_state_str(current_state_id);
                    const auto& to   = get_state_str(to_state_id);

                    result += std::format("{} --> {} : {}\n", from, to,
                                          static_cast<char>(input_char));
                }
            }

            for (auto& [state_id, input_chars] : merge_map) {
                std::string state_str = get_state_str(state_id);
                std::string input_char;

                for (char ch : input_chars) {
                    input_char += std::format("{},", ch);
                }

                input_char.pop_back();

                result +=
                    std::format("{} --> {} : {}\n", state_str, state_str, input_char);
            }

            result += "```";

            return std::format_to(ctx.out(), "{}", result);
        }
    };
} // namespace std
