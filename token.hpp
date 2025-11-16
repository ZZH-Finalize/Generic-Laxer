#pragma once

#include <format>
#include <functional>
#include <string>
#include "regex/nfa.hpp"
#include "regex_typedef.hpp"

namespace laxer {

    class token: public regex::final_state_t {
       public:
        using id_t     = regex::id_t;
        using action_t = std::function<bool(token &)>;

       private:
        id_t token_id;
        std::string matched_text, rule_name;

        action_t action;

       public:
        // 实现终态类似所必须的方法
        token(id_t state_id, id_t token_id = 0, const action_t &cb = {},
              const std::string name = {})
            : regex::final_state_t(state_id),
              token_id(token_id),
              matched_text {},
              rule_name(std::move(name)),
              action(cb)

        {
        }

        // 实现其他方法
        void add_matched_text(const std::string &text)
        {
            this->matched_text.append(text);
        }

        void add_matched_text(char text)
        {
            this->matched_text.push_back(text);
        }

        void set_token_id(id_t id) noexcept
        {
            this->token_id = id;
        }

        id_t get_token_id(void) const noexcept
        {
            return this->token_id;
        }

        void set_matched_text(const std::string &text) noexcept
        {
            this->matched_text = text;
        }

        void set_matched_text(std::string &&text) noexcept
        {
            this->matched_text = std::move(text);
        }

        const auto &get_matched_text(void) const noexcept
        {
            return this->matched_text;
        }

        void set_rule_name(const std::string &name) noexcept
        {
            this->rule_name = name;
        }

        void set_rule_name(std::string &&name) noexcept
        {
            this->rule_name = std::move(name);
        }

        const auto &get_rule_name(void) const noexcept
        {
            return this->rule_name;
        }

        void set_action(const action_t &action) noexcept
        {
            this->action = action;
        }

        const auto &get_action(void) const noexcept
        {
            return this->action;
        }
    };

} // namespace laxer

namespace std {

    template<typename CharT>
    struct formatter<laxer::token, CharT>
    {
        template<typename ParseContext>
        constexpr auto parse(ParseContext &ctx)
        {
            return ctx.begin();
        }

        template<typename FormatContext>
        auto format(const laxer::token &tok, FormatContext &ctx) const
            -> decltype(ctx.out())
        {
            return std::format_to(
                ctx.out(),
                "token(dfa_state_id={}, token_id={}, matched_text={}, rule_name={})",
                static_cast<laxer::token::id_t>(tok), tok.get_token_id(),
                tok.get_matched_text(), tok.get_rule_name());
        }
    };

} // namespace std
