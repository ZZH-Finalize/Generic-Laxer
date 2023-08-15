/*
@file: laxer.h
@author: ZZH
@date: 2023-07-27
@info: edl语言词法分析器组件
*/
#pragma once
#include <iostream>
#include <fstream>
#include <cstdint>

namespace EDL
{
    class Laxer_t
    {
    private:
        const std::size_t token_id_bits = 8;
        const std::size_t state_map_size = 1 << (token_id_bits * 2);

        std::basic_istream<char, std::char_traits<char>>& input;
        std::ostream& debug;
        bool verbose;

        uint8_t* state_map;
        static const char* end_chars;

        enum state_t
        {
            error = 255,
            end = 0,
            start = 1,
            ignore,

            number_iden,// number identifaction
            number_bin,
            number_dec,
            number_hex,
            identifer,
            string_double,
            string_single,
            operators,//+-*/ etc.
            key_word,
        };

        static const char* state_names[];

        static inline const char* get_state_names(state_t state)
        {
            if (255 == state)
                return "error";

            return state_names[state];
        }

    protected:
        void init_state_map(void);
        inline bool get_state_map(uint8_t* buf) const
        {
            if (nullptr == this->state_map)
                return false;

            memcpy(buf, this->state_map, this->state_map_size);
            return true;
        }

    public:
        typedef enum
        {
            // control token
            eof = 0,
            spacer = 1,
            ascii_end = 127,
            ascii_char_start = ' ',//32

            // ascii tokens (including a-z A-Z 0-9)
            comma = ',',
            dot = '.',
            dash = '-',
            slash = '_',

            rbrackets_left = '(',
            rbrackets_right = ')',
            brackets_left = '[',
            brackets_right = ']',
            brace_left = '{',
            brace_right = '}',

            operator_add = '+',
            operator_sub = '-',
            operator_mul = '*',
            operator_div = '/',
            operator_mod = '%',

            operator_bnot = '~',
            operator_band = '&',
            operator_bor = '|',
            operator_not = '!',//logical not

            operator_greater = '>',
            operator_less = '<',

            // outof ascii tokens
            operator_and = ascii_end + 1,//logical and
            operator_or,//logical or
            operator_equ,//logical equ

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

            //other tokens
            tk_symbol,
            tk_number,
            tk_string,

            //rev invalid token
            invalid

        } token_id_t;

        typedef struct
        {
            std::string string;
            std::size_t integer;
            double number;
        } token_value_t;

        Laxer_t(std::ifstream& i, std::ostream& o = std::cout, bool verbose = false);
        ~Laxer_t();

        inline void reset(void)
        {

        }

        inline void skip_to_next_line()
        {
            char ch = this->input.get();

            if (-1 == ch)
                return;

            while ('\n' != ch)
                ch = this->input.get();
        }

        token_id_t next_token(void);

        inline void set_verbose(bool ver = true) { this->verbose = ver; }

    private:
        token_value_t value;

    public:
        const token_value_t& get_token_value(void) const
        {
            return this->value;
        }

        void clear_token_value(void)
        {
            this->value.integer = 0;
            this->value.number = 0;
            this->value.string.clear();
        }
    };
} // namespace EDL
