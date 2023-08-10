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
        std::filebuf* input;
        std::ostream& debug;

        uint8_t* state_map;

        const char* number_ends = " \t\r\n:;,?+-*/%^&|!#<>()[]{}";

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
            character,
            identifer,
            string,
            operators,//+-*/ etc.
            key_word,
        };

    protected:
        void init_state_map(void);

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

        typedef union
        {
            std::string* string;
            std::string* symbol;
            std::int64_t integer;

        }token_value_t;

        class token_t
        {
        public:
            token_id_t id;
            token_value_t value;

            ~token_t()
            {
                if (tk_string == this->id || tk_string == this->id)
                {
                    if (nullptr != this->value.string)
                        delete this->value.symbol;
                }
            }
        };

        Laxer_t(const std::ifstream& i, std::ostream& o = std::cout);
        ~Laxer_t();

        inline void switch_to(const std::ifstream& i)
        {
            this->input = i.rdbuf();
        }

        inline void reset(void)
        {
        }

        inline void skip_to_next_line()
        {
            char ch = this->input->sbumpc();

            if (-1 == ch)
                return;

            while ('\n' != ch)
                ch = this->input->sbumpc();
        }

        token_t next_token(void);
    };
} // namespace EDL
