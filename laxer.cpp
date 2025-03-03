#include "laxer.h"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <exception>

#define for_each_ascii(varname) for (char varname = ' '; varname <= '~'; varname++)

namespace EDL {
    const char *Laxer_t::end_chars     = " \t\r\n:;,?+-*/%^&|!#<>()[]{}";
    const char *Laxer_t::state_names[] = {
        "end",           "start",         "ignore",     "number_iden",
        "number_bin",    "number_dec",    "number_hex", "identifer",
        "string_double", "string_single", "operators",  "key_word",
    };

    Laxer_t::Laxer_t(std::ifstream &i, std::ostream &o, bool verbose)
        : input(i), debug(o), verbose(verbose)
    {
        this->init_state_map();
        this->debug << "new laxer" << std::endl;
    }

    Laxer_t::~Laxer_t()
    {
        if (nullptr != this->state_map) delete[] this->state_map;
    }

    Laxer_t::token_id_t Laxer_t::next_token(void)
    {
        assert(nullptr != this->state_map);

        token_id_t token;
        state_t cur_state = start;
        this->clear_token_value();

        do {
            if (true == this->input.eof()) {
                token = eof;
                return token;
            }

            char ch = this->input.get();
            auto next_state =
                static_cast<state_t>(this->state_map[make_id(cur_state, ch)]);

            if (this->verbose) {
                this->debug << "read a char: " << ch << " ("
                            << reinterpret_cast<void *>(ch) << ")\n";
                this->debug << "cur_state: " << this->get_state_names(cur_state) << " -> "
                            << this->get_state_names(next_state) << "\n"
                            << std::endl;
            }

            cur_state = next_state;

            switch (cur_state) {
                case end: this->input.putback(ch); break;

                case number_iden: token = tk_number; break;

                case number_bin:
                    if (ch == 'b') break;
                    this->value.integer <<= 1;
                    this->value.integer |= ch == '1';
                    break;

                case number_dec:
                    token = tk_number;
                    this->value.integer *= 10;
                    this->value.integer += ch - '0';
                    break;

                case number_hex:
                    if (ch == 'x') break;

                    this->value.integer <<= 4;
                    if (ch >= 'a' && ch <= 'f')
                        this->value.integer |= (ch - 'a') + 10;
                    else if (ch >= 'A' && ch <= 'F')
                        this->value.integer |= (ch - 'A') + 10;
                    else
                        this->value.integer |= ch - '0';

                    break;

                case string_single:
                case string_double:
                    if ('"' == ch || '\'' == ch) {
                        token = tk_string;
                        break;
                    }
                case identifer:
                    if (tk_string != token) token = tk_symbol;

                    this->value.string.push_back(ch);
                    break;
            }
        } while (cur_state != error && cur_state != end);

        if (cur_state == error) {
            token = invalid;
        }

        return token;
    }

    void Laxer_t::init_state_map(void)
    {
        this->state_map = new std::uint8_t[this->state_map_size];
        memset(this->state_map, error, this->state_map_size);
        // 0b01101001
        // 0xabcd1234
        // 012345
        //_abcde_45
        // abcde__
        //_____
        //"abcde"

        // for (char ch = ' ';ch <= '\b';ch++)
        //     this->add_rule(start,  ch, ch);

        // start : 0 -> number_iden
        this->add_rule(start,  '0', number_iden);
        // start : 1-9 -> number_dec
        for (char ch = '1'; ch <= '9'; ch++)
            this->add_rule(start,  ch, number_dec);

        // a-z
        for (char ch = 'a'; ch <= 'z'; ch++) {
            // start : a-z -> identifer
            this->add_rule(start,  ch, identifer);

            // identifer : a-z -> identifer
            this->add_rule(identifer,  ch, identifer);

            // _ : a-z -> identifer
            this->add_rule('_',  ch, identifer);

            // string_double : a-z -> string_double
            this->add_rule(string_double,  ch, string_double);
        }

        // A-Z
        for (char ch = 'A'; ch <= 'Z'; ch++) {
            // start : A-Z -> identifer
            this->add_rule(start,  ch, identifer);

            // identifer : A-Z -> identifer
            this->add_rule(identifer,  ch, identifer);

            // _ : A-Z -> identifer
            this->add_rule('_',  ch, identifer);

            // string_double : A-Z -> string_double
            this->add_rule(string_double,  ch, string_double);
        }

        // start : _ -> identifer
        this->add_rule(start,  '_', identifer);
        // identifer : _ -> identifer
        this->add_rule(identifer,  '_', identifer);

        // start : " -> string_double
        this->add_rule(start,  '"', string_double);
        // start : " -> string_single
        this->add_rule(start,  '\'', string_single);

        // 0-9
        for (char ch = '0'; ch <= '9'; ch++) {
            // nummber_iden : 0 -> number_dec
            this->add_rule(number_iden,  ch, number_dec);

            // number_dec : 0-9 -> number_dec
            this->add_rule(number_dec,  ch, number_dec);

            // number_hex : 0-9 -> number_hex
            this->add_rule(number_hex,  ch, number_hex);

            // identifer : 0-9 -> identifer
            this->add_rule(identifer,  ch, identifer);
        }

        // a-f
        for (char ch = 'a'; ch <= 'f'; ch++)
            this->add_rule(number_hex,  ch, number_hex);

        // A-F
        for (char ch = 'A'; ch <= 'F'; ch++)
            this->add_rule(number_hex,  ch, number_hex);

        // number_iden : x -> number_hex
        this->add_rule(number_iden,  'x', number_hex);
        this->add_rule(number_iden,  'b', number_bin);

        // number_bin : 0-1 -> number_bin
        this->add_rule(number_bin,  '0', number_bin);
        this->add_rule(number_bin,  '1', number_bin);

        // string_[double|single] : all ascii -> string_[double|single]
        for_each_ascii(ch)
        {
            this->add_rule(string_double,  ch, string_double);
            this->add_rule(string_single,  ch, string_single);
        } // NOTE: for " and ', will override it later.

        // ends
        for (const char *ch = this->end_chars; *ch != '\0'; ch++) {
            this->add_rule(number_dec,  *ch, end);
            this->add_rule(number_hex,  *ch, end);
            this->add_rule(number_bin,  *ch, end);

            this->add_rule(identifer,  *ch, end);

            this->add_rule(start,  *ch, operators);
        }

        // string_double end
        this->add_rule(string_double,  '"', end);

        // string_single end
        this->add_rule(string_single,  '\'', end);

        /*********** ignores ***********/
        for_each_ascii(ch) this->add_rule(ignore,  ch, start);

        const char *ignore_headers                              = " \t\r\n;";
        for (const char *ch = ignore_headers; *ch != '\0'; ch++) {
            this->add_rule(start,  *ch, ignore);
            this->add_rule(ignore,  *ch, ignore);
        }

        for (char ch = 'a'; ch <= 'z'; ch++)
            this->add_rule(ignore,  ch, identifer);

        for (char ch = 'A'; ch <= 'Z'; ch++)
            this->add_rule(ignore,  ch, identifer);

        this->add_rule(ignore,  '"', string_double);
        this->add_rule(ignore,  '\'', string_single);
        this->add_rule(ignore,  '_', identifer);
        this->add_rule(ignore,  '0', number_iden);

        for (char ch = '1'; ch <= '9'; ch++)
            this->add_rule(ignore,  ch, number_dec);
    }
} // namespace EDL
