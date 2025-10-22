/*
@file: laxer.h
@author: ZZH
@date: 2023-07-27
@info: edl语言词法分析器组件
*/
#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <type_traits>
#include <variant>

namespace EDL {
    template<typename T>
    concept token_value_types =
        std::is_same_v<T, std::string> || std::is_same_v<T, std::uint64_t>
        || std::is_same_v<T, double>;

    class Token_t {
       public:
        typedef enum
        {
            // control token
            eof              = 0,
            spacer           = 1,
            ascii_end        = 127,
            ascii_char_start = ' ', // 32

            // ascii tokens (including a-z A-Z 0-9)
            comma = ',',
            dot   = '.',
            dash  = '-',
            slash = '_',

            rbrackets_left  = '(',
            rbrackets_right = ')',
            brackets_left   = '[',
            brackets_right  = ']',
            brace_left      = '{',
            brace_right     = '}',

            operator_add = '+',
            operator_sub = '-',
            operator_mul = '*',
            operator_div = '/',
            operator_mod = '%',

            operator_bnot = '~',
            operator_band = '&',
            operator_bor  = '|',
            operator_not  = '!', // logical not

            operator_greater = '>',
            operator_less    = '<',

            // outof ascii tokens
            operator_and = ascii_end + 1, // logical and
            operator_or,                  // logical or
            operator_equ,                 // logical equ

            // key words
            kw_function,
            kw_if,
            kw_else,
            kw_elif,
            kw_for,
            kw_while,
            kw_request,
            kw_until,
            kw_loop,

            // other tokens
            tk_symbol,
            tk_number,
            tk_integer,
            tk_string,

            // rev invalid token
            invalid

        } id_t;

        using value_t = std::variant<std::monostate, std::string, std::uint64_t, double>;

       private:
        id_t __id;
        value_t __value;

       public:
        explicit Token_t(const id_t &id = invalid, value_t &&value = std::monostate())
            : __id(id), __value(value)
        {
        }

        inline id_t id(void) const
        {
            return this->__id;
        }

        template<token_value_types T>
        T value(void) const
        {
            return std::get<T>(this->__value);
        }

        inline operator id_t() const
        {
            return this->id();
        }
    };

    class Laxer_t {
       private:
        const std::size_t token_id_bits  = 8;
        const std::size_t state_map_size = 1 << (token_id_bits * 2);

        std::basic_istream<char, std::char_traits<char>> &input;
        std::ostream &debug;
        bool verbose;

        uint8_t *state_map;
        static const char *end_chars;

        enum state_t
        {
            error = 255,
            end   = 0,
            start = 1,
            ignore,

            number_iden, // number identifaction
            integer_bin,
            integer_dec,
            integer_hex,
            identifer,
            string_double,
            string_single,
            operators, //+-*/ etc.
            keyword,
        };

        static const char *state_names[];

        static inline const char *get_state_names(state_t state)
        {
            if (255 == state) return "error";

            return state_names[state];
        }

       protected:
        void init_state_map(void);

        inline bool get_state_map(uint8_t *buf) const
        {
            if (nullptr == this->state_map) return false;

            memcpy(buf, this->state_map, this->state_map_size);
            return true;
        }

        static inline constexpr uint16_t make_id(uint16_t state, char ch)
        {
            return (((uint16_t) (state)) << 8 | ((uint8_t) ch));
        }

        inline void add_rule(uint16_t when, char ch, state_t switch_to)
        {
            this->state_map[this->make_id(when, ch)] = switch_to;
        }

       public:
        Laxer_t(std::ifstream &i, std::ostream &o = std::cout, bool verbose = false);
        ~Laxer_t();

        inline void reset(void)
        {
        }

        inline void skip_to_next_line()
        {
            char ch = this->input.get();

            if (-1 == ch) return;

            while ('\n' != ch) ch = this->input.get();
        }

        inline void set_verbose(bool ver = true)
        {
            this->verbose = ver;
        }

        Token_t next_token(void);
    };
} // namespace EDL
