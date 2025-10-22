#include "laxer.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <exception>
#include <string>
#include <utility>
#include <variant>

#define foreach_ascii(varname)    for (char varname = ' '; varname <= '~'; varname++)
#define foreach_ab(varname, a, b) for (char varname = a; varname <= b; varname++)

namespace EDL {
    const char *Laxer_t::end_chars     = " \t\r\n:;,?+-*/%^&|!#<>()[]{}";
    const char *Laxer_t::state_names[] = {
        "end",           "start",         "ignore",      "number_iden",
        "integer_bin",   "integer_dec",   "integer_hex", "identifer",
        "string_double", "string_single", "operators",   "key_word",
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

    Token_t Laxer_t::next_token(void)
    {
        assert(nullptr != this->state_map);

        Token_t::id_t id;
        Token_t::value_t value;

        state_t cur_state = start, last_state = start;

        do {
            // end of file
            if (true == this->input.eof()) {
                id = Token_t::eof;
                return std::move(Token_t(id));
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

            // 切换variant的类型
            if (cur_state != last_state) {
                switch (cur_state) {
                    case integer_bin:
                    case integer_dec:
                    case integer_hex:
                    case number_iden: value = static_cast<std::uint64_t>(0); break;

                    case string_single:
                    case string_double:
                    case identifer: value = ""; break;

                    default: break;
                }
            }

            switch (cur_state) {
                case start: break;
                case ignore: break;

                case keyword: break;
                case operators: break;
                case error: break;

                case end: this->input.putback(ch); break;

                case number_iden: break;

                case integer_bin: {
                    if (ch == 'b') break;
                    auto old_val = std::get<std::uint64_t>(value);

                    id    = Token_t::tk_integer;
                    value = (old_val << 1) | (ch == '1');
                } break;

                case integer_dec: {
                    auto old_val = std::get<std::uint64_t>(value);

                    id    = Token_t::tk_integer;
                    value = (old_val * 10) + (ch - '0');
                } break;

                case integer_hex: {
                    if (ch == 'x') break;
                    auto old_val = std::get<std::uint64_t>(value);

                    old_val <<= 4;
                    if (ch >= 'a' && ch <= 'f')
                        old_val |= (ch - 'a') + 10;
                    else if (ch >= 'A' && ch <= 'F')
                        old_val |= (ch - 'A') + 10;
                    else
                        old_val |= ch - '0';

                    id    = Token_t::tk_integer;
                    value = old_val;
                } break;

                case string_single:
                case string_double:
                    if ('"' == ch || '\'' == ch) {
                        id = Token_t::tk_string;
                        break;
                    }
                case identifer:
                    if (Token_t::tk_string != id) id = Token_t::tk_symbol;

                    std::get<std::string>(value).push_back(ch);
                    break;
            }

            last_state = cur_state;
        } while (cur_state != error && cur_state != end);

        if (cur_state == error) {
            id = Token_t::invalid;
        }

        return Token_t(id, std::move(value));
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

        // numbers
        // start : 0 -> number_iden
        this->add_rule(start, '0', number_iden);
        // start : 1-9 -> integer_dec
        foreach_ab (ch, '1', '9') this->add_rule(start, ch, integer_dec);

        // return;

        // a-z
        foreach_ab (ch, 'a', 'z') {
            // start : a-z -> identifer
            this->add_rule(start, ch, identifer);

            // identifer : a-z -> identifer
            this->add_rule(identifer, ch, identifer);

            // _ : a-z -> identifer
            this->add_rule('_', ch, identifer);

            // string_double : a-z -> string_double
            this->add_rule(string_double, ch, string_double);
        }

        // A-Z
        foreach_ab (ch, 'A', 'Z') {
            // start : A-Z -> identifer
            this->add_rule(start, ch, identifer);

            // identifer : A-Z -> identifer
            this->add_rule(identifer, ch, identifer);

            // _ : A-Z -> identifer
            this->add_rule('_', ch, identifer);

            // string_double : A-Z -> string_double
            this->add_rule(string_double, ch, string_double);
        }

        // start : _ -> identifer
        this->add_rule(start, '_', identifer);
        // identifer : _ -> identifer
        this->add_rule(identifer, '_', identifer);

        // start : " -> string_double
        this->add_rule(start, '"', string_double);
        // start : " -> string_single
        this->add_rule(start, '\'', string_single);

        // 0-9
        foreach_ab (ch, '0', '9') {
            // nummber_iden : 0 -> integer_dec
            this->add_rule(number_iden, ch, integer_dec);

            // integer_dec : 0-9 -> integer_dec
            this->add_rule(integer_dec, ch, integer_dec);

            // integer_hex : 0-9 -> integer_hex
            this->add_rule(integer_hex, ch, integer_hex);

            // identifer : 0-9 -> identifer
            this->add_rule(identifer, ch, identifer);
        }

        // a-f
        foreach_ab (ch, 'a', 'f') this->add_rule(integer_hex, ch, integer_hex);

        // A-F
        foreach_ab (ch, 'A', 'F') this->add_rule(integer_hex, ch, integer_hex);

        // number_iden : x -> integer_hex
        this->add_rule(number_iden, 'x', integer_hex);
        this->add_rule(number_iden, 'b', integer_bin);

        // integer_bin : 0-1 -> integer_bin
        this->add_rule(integer_bin, '0', integer_bin);
        this->add_rule(integer_bin, '1', integer_bin);

        // string_[double|single] : all ascii -> string_[double|single]
        foreach_ascii (ch) {
            this->add_rule(string_double, ch, string_double);
            this->add_rule(string_single, ch, string_single);
        } // NOTE: for " and ', will override it later.

        // ends
        for (const char *ch = this->end_chars; *ch != '\0'; ch++) {
            this->add_rule(integer_dec, *ch, end);
            this->add_rule(integer_hex, *ch, end);
            this->add_rule(integer_bin, *ch, end);

            this->add_rule(identifer, *ch, end);

            this->add_rule(start, *ch, operators);
        }

        // string_double end
        this->add_rule(string_double, '"', end);

        // string_single end
        this->add_rule(string_single, '\'', end);

        /*********** ignores ***********/
        foreach_ascii (ch) this->add_rule(ignore, ch, start);

        const char *ignore_headers = " \t\r\n;";
        for (const char *ch = ignore_headers; *ch != '\0'; ch++) {
            this->add_rule(start, *ch, ignore);
            this->add_rule(ignore, *ch, ignore);
        }

        for (char ch = 'a'; ch <= 'z'; ch++) this->add_rule(ignore, ch, identifer);

        for (char ch = 'A'; ch <= 'Z'; ch++) this->add_rule(ignore, ch, identifer);

        this->add_rule(ignore, '"', string_double);
        this->add_rule(ignore, '\'', string_single);
        this->add_rule(ignore, '_', identifer);
        this->add_rule(ignore, '0', number_iden);

        for (char ch = '1'; ch <= '9'; ch++) this->add_rule(ignore, ch, integer_dec);
    }
} // namespace EDL
