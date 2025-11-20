#pragma once

#include <format>
#include <set>
#include <string>

#include "regex/nfa.hpp"
#include "regex_typedef.hpp"
#include "token.hpp"

namespace laxer {
    class final_state_t {
       public:
        using id_t = regex::id_t;

       private:
        id_t state_id;
        std::string rule_name;
        // std::function<> action;

       public:
    };

    class nfa: public regex::basic_nfa<std::set<token>> {
       public:
        using final_state_id_t = typename basic_nfa::final_state_id_t;

        nfa()
        {
        }

        // 添加单个regex::nfa到当前NFA
        void add_nfa(const regex::nfa& input_nfa, id_t token_id = 0,
                     const token::action_t& cb = {}, std::string name = {})
        {
            auto offset = this->merge_states(input_nfa);

            // 从起始状态创建epsilon转换到新添加NFA的起始状态
            this->add_epsilon_transition(this->get_start(),
                                         input_nfa.get_start() + offset);

            // 自动生成规则名
            if (name.empty()) {
                name = std::format("rule_{}", token_id);
            }

            // 把input_nfa的终态添加到accept_states中, 并填充元数据
            this->add_final(input_nfa.get_final() + offset, token_id, cb, name);
        }

        std::optional<final_state_id_t> find_final(const closure_t& states) const
        {
            for (const auto& state : states) {
                const auto& final = this->get_final().find(state);
                if (final != this->get_final().end()) {
                    return *final;
                }
            }

            return std::nullopt;
        }
    };

} // namespace laxer

namespace std {
    template<typename CharT>
    struct formatter<laxer::nfa, CharT>
        : formatter<regex::basic_nfa<laxer::nfa::final_state_t>, CharT>
    {
        using formatter<regex::basic_nfa<laxer::nfa::final_state_t>, CharT>::parse;

        // using formatter<regex::basic_nfa<laxer::nfa::final_state_t>, CharT>::format;

        string get_state_str(const laxer::nfa& nfa, regex::id_t id) const
        {
            auto final = nfa.find_final({id});

            if (id == nfa.get_start()) {
                return std::format("start_{}", id);
            } else if (final.has_value()) {
                return std::format("final_{}", id);
            }

            return std::format("state_{}", id);
        }

        template<typename FormatContext>
        auto format(const laxer::nfa& nfa, FormatContext& ctx) const
        {
            std::string result = "```mermaid\n";

            result += "stateDiagram-v2\n";

            if (not this->direction.empty()) {
                result += std::format("direction {}\n", this->direction);
            }

            result += "\n";

            for (const auto& final_state : nfa.get_final()) {
                regex::id_t state_id = final_state;

                auto key = this->get_state_str(nfa, state_id);

                result +=
                    std::format("{}: accept rule {}\n", key, final_state.get_rule_name());
            }

            auto& states = nfa.get_states();

            for (size_t current_state_id = 0; current_state_id < states.size();
                 current_state_id++) {
                auto final =
                    nfa.find_final({static_cast<laxer::nfa::id_t>(current_state_id)});
                if (final.has_value()) {
                    continue;
                }

                const laxer::nfa::state& state = states.at(current_state_id);
                std::string from = this->get_state_str(nfa, current_state_id);

                // handle epsilon transitions
                for (auto& to_state : state.get_epsilon_transition()) {
                    std::string to = this->get_state_str(nfa, to_state);

                    result += std::format("{} --> {}\n", from, to);
                }

                // handle char transitions
                auto trans_map = state.get_transition_map();
                // 遍历所有可能的输入
                for (auto input = 0; input < trans_map.size(); input++) {
                    auto& trans = trans_map.at(input);

                    // 对输入字符, 遍历可能的目标状态
                    for (auto& to_state : trans) {
                        std::string to = this->get_state_str(nfa, to_state);

                        result += std::format("{} --> {} : {}\n", from, to,
                                              static_cast<char>(input));
                    }
                }
            }

            result += "```";

            return std::format_to(ctx.out(), "{}", result);
        }
    };
} // namespace std
