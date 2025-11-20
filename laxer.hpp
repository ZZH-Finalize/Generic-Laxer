#pragma once

#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>
#include "nfa.hpp"
#include "regex/regex.hpp"
#include "token.hpp"

namespace laxer {

    class laxer {
       protected:
        std::ifstream src_file;
        nfa nfa;
        nfa::dfa dfa;

       public:
        class laxer_error: public std::runtime_error {
           public:
            explicit laxer_error(const std::string& msg): std::runtime_error(msg)
            {
            }
        };

        laxer(): src_file {}, nfa {}, dfa {}
        {
        }

        inline void open_file(const std::string& fn)
        {
            this->src_file.open(fn, std::ios::in);
        }

        inline void add_rule(const std::string& regex, nfa::id_t token_id = 0,
                             const token::action_t& cb = {}, std::string name = {})
        {
            this->nfa.add_nfa(regex::build_nfa(regex), token_id, cb, name);
        }

        inline void generate_dfa(void)
        {
            this->dfa = regex::build(this->nfa);
        }

        token next_token(void)
        {
            if (this->dfa.get_states().empty()) {
                this->generate_dfa();

                if (this->dfa.get_states().empty()) {
                    throw laxer_error("no valid rules");
                }
            }

            const auto& transition_map = this->dfa.get_states();
            const auto& final_states   = this->dfa.get_final();

            token cur_token(0, nfa::invalid_state);
            bool is_return = false;

            while (not is_return) {
                std::string matched_text;
                nfa::dfa::id_t current_state = this->dfa.get_start();

                // 持续匹配, 直到遇到无效状态
                while (current_state != nfa::invalid_state) {
                    // 根据输入字符推动状态转换
                    char current_char = this->src_file.peek();
                    // 文件结束
                    if (this->src_file.eof()) {
                        return token(nfa::invalid_state, nfa::invalid_state, {}, "EOF");
                    }

                    auto next_state =
                        transition_map[current_state].get_transition(current_char);

                    // 匹配到某条规则
                    auto it = final_states.find(next_state);
                    if (it != final_states.end()) {
                        // 记录匹配到的token
                        if (it->get_token_id() != cur_token.get_token_id()) {
                            cur_token = *it;
                        }
                    }

                    // 下一状态不是无效状态
                    if (next_state != nfa::invalid_state) {
                        matched_text.push_back(current_char);
                        this->src_file.get();
                    } else if (current_state == this->dfa.get_start()
                               and next_state == nfa::invalid_state) {
                        token default_token(nfa::invalid_state, nfa::invalid_state, {}, "default");
                        matched_text.push_back(current_char);
                        default_token.set_matched_text(std::move(matched_text));
                        return default_token;
                    }

                    current_state = next_state;
                }

                // 设置与token对应的文本
                cur_token.set_matched_text(std::move(matched_text));

                // 调用回调来决定是否要返回这个token
                auto action = cur_token.get_action();
                if (action) {
                    is_return = action(cur_token);
                } else {
                    // 没有写回调的情况下默认需要return;
                    is_return = true;
                }
            }

            return cur_token;
        }
    };

} // namespace laxer
