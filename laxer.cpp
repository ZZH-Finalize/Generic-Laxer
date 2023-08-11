#include <exception>
#include <cstring>
#include <cassert>
#include "laxer.h"

#define make_id(a,b) (((uint16_t)(a))<<8|((uint8_t)b))

namespace EDL
{
    Laxer_t::Laxer_t(std::ifstream& i, std::ostream& o)
        :input(i), debug(o)
    {

        this->init_state_map();
        this->debug << "new laxer" << std::endl;
    }

    Laxer_t::~Laxer_t()
    {
        if (nullptr != this->state_map)
            delete[] this->state_map;
    }

    Laxer_t::token_t Laxer_t::next_token(void)
    {
        assert(nullptr != this->state_map);

        token_t token;
        state_t cur_state = start;
        memset(&token, 0, sizeof(token));

        do
        {
            if (true == this->input.eof())
            {
                token.id = eof;
                return token;
            }

            char ch = this->input.get();
            cur_state = (state_t) this->state_map[make_id(cur_state, ch)];

            switch (cur_state)
            {
                case end:
                    break;

                case number_iden:
                    token.id = tk_number;
                    break;

                case number_bin:
                    if (ch == 'b')
                        break;
                    token.value.integer <<= 1;
                    token.value.integer |= ch == '1';
                    break;

                case number_dec:
                    token.id = tk_number;
                    token.value.integer *= 10;
                    token.value.integer += ch - '0';
                    break;

                case number_hex:
                    if (ch == 'x')
                        break;

                    token.value.integer <<= 4;
                    if (ch >= 'a' && ch <= 'f')
                        token.value.integer |= (ch - 'a') + 10;
                    else if (ch >= 'A' && ch <= 'F')
                        token.value.integer |= (ch - 'A') + 10;
                    else
                        token.value.integer |= ch - '0';

                    break;

                case string_single:
                case string_double:
                    if ('"' == ch || '\'' == ch)
                    {
                        token.id = tk_string;
                        break;
                    }
                case identifer:
                    if (nullptr == token.value.symbol)
                    {
                        token.value.symbol = new std::string;
                        if (tk_string != token.id)
                            token.id = tk_symbol;
                    }

                    token.value.symbol->push_back(ch);
                    break;

            }
        } while (cur_state != error && cur_state != end);

        if (cur_state == error)
        {
            if (tk_string == token.id || tk_symbol == token.id)
                delete token.value.symbol;

            token.id = invalid;
            token.value.integer = 0;
        }

        return token;
    }

    void Laxer_t::init_state_map(void)
    {
        this->state_map = new std::uint8_t[65535];
        memset(this->state_map, error, 65535);
        //0b01101001
        //0xabcd1234
        //012345
        //_abcde_45
        //abcde__
        //_____
        //"abcde"

        // for (char ch = ' ';ch <= '\b';ch++)
        //     this->state_map[make_id(start, ch)] = ch;

        // start : 0 -> number_iden
        this->state_map[make_id(start, '0')] = number_iden;
        // start : 1-9 -> number_dec
        for (char ch = '1';ch <= '9';ch++)
            this->state_map[make_id(start, ch)] = number_dec;

        // a-z
        for (char ch = 'a';ch <= 'z';ch++)
        {
            // start : a-z -> identifer
            this->state_map[make_id(start, ch)] = identifer;

            // identifer : a-z -> identifer
            this->state_map[make_id(identifer, ch)] = identifer;

            // _ : a-z -> identifer
            this->state_map[make_id('_', ch)] = identifer;

            // string_double : a-z -> string_double
            this->state_map[make_id(string_double, ch)] = string_double;
        }

        // A-Z
        for (char ch = 'A';ch <= 'Z';ch++)
        {
            // start : A-Z -> identifer
            this->state_map[make_id(start, ch)] = identifer;

            // identifer : A-Z -> identifer
            this->state_map[make_id(identifer, ch)] = identifer;

            // _ : A-Z -> identifer
            this->state_map[make_id('_', ch)] = identifer;

            // string_double : A-Z -> string_double
            this->state_map[make_id(string_double, ch)] = string_double;
        }

        // start : _ -> identifer
        this->state_map[make_id(start, '_')] = identifer;
        // identifer : _ -> identifer
        this->state_map[make_id(identifer, '_')] = identifer;

        // start : " -> string_double
        this->state_map[make_id(start, '"')] = string_double;
        // start : " -> string_single
        this->state_map[make_id(start, '\'')] = string_single;

        // 0-9
        for (char ch = '0';ch <= '9';ch++)
        {
            // nummber_iden : 0 -> number_dec
            this->state_map[make_id(number_iden, ch)] = number_dec;

            // number_dec : 0-9 -> number_dec
            this->state_map[make_id(number_dec, ch)] = number_dec;

            // number_hex : 0-9 -> number_hex
            this->state_map[make_id(number_hex, ch)] = number_hex;

            // identifer : 0-9 -> identifer
            this->state_map[make_id(identifer, ch)] = identifer;
        }

        // a-f
        for (char ch = 'a';ch <= 'f';ch++)
            this->state_map[make_id(number_hex, ch)] = number_hex;

        // A-F
        for (char ch = 'A';ch <= 'F';ch++)
            this->state_map[make_id(number_hex, ch)] = number_hex;

        // number_iden : x -> number_hex
        this->state_map[make_id(number_iden, 'x')] = number_hex;
        this->state_map[make_id(number_iden, 'b')] = number_bin;

        // number_bin : 0-1 -> number_bin
        this->state_map[make_id(number_bin, '0')] = number_bin;
        this->state_map[make_id(number_bin, '1')] = number_bin;

        // string_[double|single] : all ascii -> string_[double|single]
        for (char ch = ' ';ch <= '~';ch++)
        {
            this->state_map[make_id(string_double, ch)] = string_double;
            this->state_map[make_id(string_single, ch)] = string_single;
        }// NOTE: for " and ', will override it later.

        // ends
        for (const char* ch = this->number_ends;*ch != '\0';ch++)
        {
            this->state_map[make_id(number_dec, *ch)] = end;
            this->state_map[make_id(number_hex, *ch)] = end;
            this->state_map[make_id(number_bin, *ch)] = end;

            this->state_map[make_id(identifer, *ch)] = end;
        }

        // string_double end
        this->state_map[make_id(string_double, '"')] = end;

        // string_single end
        this->state_map[make_id(string_single, '\'')] = end;

        /*********** ignores ***********/
        this->state_map[make_id(start, ' ')] = ignore;
        this->state_map[make_id(ignore, ' ')] = ignore;
        this->state_map[make_id(start, '\t')] = ignore;
        this->state_map[make_id(ignore, '\t')] = ignore;
        this->state_map[make_id(start, '\r')] = ignore;
        this->state_map[make_id(ignore, '\r')] = ignore;
        this->state_map[make_id(start, '\n')] = ignore;
        this->state_map[make_id(ignore, '\n')] = ignore;

        this->state_map[make_id(start, ';')] = ignore;
        this->state_map[make_id(ignore, ';')] = start;// back to start

        for (char ch = 'a';ch <= 'z';ch++)
            this->state_map[make_id(ignore, ch)] = identifer;

        for (char ch = 'A';ch <= 'Z';ch++)
            this->state_map[make_id(ignore, ch)] = identifer;

        this->state_map[make_id(ignore, '"')] = string_double;
        this->state_map[make_id(ignore, '\'')] = string_single;
        this->state_map[make_id(ignore, '_')] = identifer;
        this->state_map[make_id(ignore, '0')] = number_iden;

        for (char ch = '1';ch <= '9';ch++)
            this->state_map[make_id(ignore, ch)] = number_dec;
    }
} // namespace EDL

