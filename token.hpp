#pragma once

#include <cstdint>
#include <format>
#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include "regex/nfa.hpp"
#include "regex_typedef.hpp"

#define STATIC_CONVERT(fn)                \
    static bool fn(token &token) noexcept \
    {                                     \
        token.convert_##fn();             \
        return true;                      \
    }

namespace laxer {

    class token: public regex::final_state_t {
       public:
        using id_t     = regex::id_t;
        using action_t = std::function<bool(token &)>;

       private:
        id_t token_id;
        std::string matched_text, rule_name;

        action_t action;
        std::variant<std::monostate, std::int32_t, std::uint32_t, std::int64_t,
                     std::uint64_t, double, std::string>
            token_value;

       public:
        // 实现终态类所必须的方法
        token(id_t state_id = 0, id_t token_id = 0, const action_t &cb = {},
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

        template<typename T>
        void set_token_value(const T &val)
        {
            this->token_value = val;
        }

        const auto &get_token_value(void) const noexcept
        {
            return this->token_value;
        }

        // token value converters

        // accept format 0b[01]+
        void convert_bin(void) noexcept
        {
            std::string_view matched_str(this->matched_text);
            matched_str.remove_prefix(2);

            std::uint32_t value = 0;

            for (char ch : matched_str) {
                value <<= 1;
                value |= (ch == '1');
            }

            this->token_value = value;
        }

        // accept format \d+
        void convert_dec(void) noexcept
        {
            this->token_value = static_cast<std::int32_t>(std::stoi(this->matched_text));
        }

        // accept format 0x[a-fA-F0-9]+
        void convert_hex(void) noexcept
        {
            std::string_view matched_str(this->matched_text);
            matched_str.remove_prefix(2);

            std::uint32_t value = 0;

            for (char ch : matched_str) {
                value <<= 4;
                if (ch >= '0' && ch <= '9') {
                    value |= (ch - '0');
                } else if (ch >= 'a' && ch <= 'f') {
                    value |= (ch - 'a' + 10);
                } else if (ch >= 'A' && ch <= 'F') {
                    value |= (ch - 'A' + 10);
                }
            }

            this->token_value = value;
        }

        // accept format \d+\.\d+
        void convert_fp64(void) noexcept
        {
            this->token_value = std::stod(this->matched_text);
        }

        void record_string(void) noexcept
        {
            this->token_value = this->matched_text;
        }
    };

    class converter {
       public:
        STATIC_CONVERT(bin);
        STATIC_CONVERT(dec);
        STATIC_CONVERT(hex);
        STATIC_CONVERT(fp64);

        static bool string(token &token) noexcept
        {
            token.record_string();

            return true;
        }

        static bool ignore(token &token) noexcept
        {
            return false;
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
