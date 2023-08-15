#include <iostream>
#include <fstream>
#include "laxer.h"

int main(const int argc, const char** argv)
{
    std::ifstream test_file(argv[1]);

    if (false == test_file.is_open())
    {
        std::cout << "file open fail" << std::endl;
        return -1;
    }

    EDL::Laxer_t laxer(test_file, std::cout, false);
    EDL::Laxer_t::token_id_t token;

    do
    {
        token = laxer.next_token();

        switch (token)
        {
            case EDL::Laxer_t::invalid: {
                std::cout << "identifer error, skip to next line" << std::endl;
                laxer.skip_to_next_line();
            } break;

            case EDL::Laxer_t::eof: {
                std::cout << "achieve the end of the file" << std::endl;
            } break;

            case EDL::Laxer_t::tk_number: {
                auto number = laxer.get_token_value().integer;
                std::cout << "value (dec): " << number << ", value (hex): " << (void*) number << std::endl;
            } break;

            case EDL::Laxer_t::tk_symbol: {
                const auto& sym = laxer.get_token_value().string;
                std::cout << "identifer: " << sym << std::endl;
            } break;

            case EDL::Laxer_t::tk_string: {
                const auto& str = laxer.get_token_value().string;
                std::cout << "string: " << str << std::endl;
            } break;

            case ' ' ... '~': {
                std::cout << "ascii char: " << (char) token << std::endl;
            } break;

            default: {
                std::cerr << "invalid type: " << token << std::endl;
            } break;
        }

    } while (token != EDL::Laxer_t::eof);

    test_file.close();

    return 0;
}
