#include <exception>
#include <cstring>
#include <cassert>
#include "laxer.h"

#define make_id(a,b) (((uint16_t)(a))<<8|((uint8_t)b))

namespace EDL
{
    Laxer_t::Laxer_t(const std::istream& i, std::ostream& o)
        :input(i.rdbuf()), debug(o), cur_state(start)
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
        assert(nullptr != this->input);
        token_t token;
        memset(&token, 0, sizeof(token));

        do
        {
            char ch = this->input->sbumpc();
            this->cur_state = (state_t) this->state_map[make_id(this->cur_state, ch)];

            switch (this->cur_state)
            {
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
            }
        } while (cur_state != error && cur_state != end);

        if (cur_state == error)
            token.id = invalid;

        this->cur_state = start;

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

        // start : 0 -> number_iden
        this->state_map[make_id(start, '0')] = number_iden;
        // start : 1-9 -> number_dec
        for (char ch = '1';ch <= '9';ch++)
            this->state_map[make_id(start, ch)] = number_dec;

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

            // string : 0-9 -> string
            this->state_map[make_id(string, ch)] = string;
        }

        //a-f
        for (char ch = 'a';ch <= 'f';ch++)
            this->state_map[make_id(number_hex, ch)] = number_hex;

        //A-F
        for (char ch = 'A';ch <= 'F';ch++)
            this->state_map[make_id(number_hex, ch)] = number_hex;

        // number_iden : x -> number_hex
        this->state_map[make_id(number_iden, 'x')] = number_hex;
        this->state_map[make_id(number_iden, 'b')] = number_bin;

        // number_bin : 0-1 -> number_bin
        this->state_map[make_id(number_bin, '0')] = number_bin;
        this->state_map[make_id(number_bin, '1')] = number_bin;

        for (const char* ch = this->number_ends;*ch != '\0';ch++)
        {
            this->state_map[make_id(number_dec, *ch)] = end;
            this->state_map[make_id(number_hex, *ch)] = end;
            this->state_map[make_id(number_bin, *ch)] = end;
        }
    }
} // namespace EDL

