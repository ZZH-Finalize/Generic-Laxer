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

    EDL::Laxer_t laxer(test_file);
    EDL::Laxer_t::token_t token;

    do
    {
        token = laxer.next_token();

        if (EDL::Laxer_t::invalid == token.id)
        {
            std::cout << "identifer error, skip to next line" << std::endl;
            laxer.skip_to_next_line();
            continue;
        }
        else if (EDL::Laxer_t::eof == token.id)
        {
            std::cout << "achieve the end of the file" << std::endl;
            break;
        }
        else if (EDL::Laxer_t::tk_number == token.id)
        {
            std::cout << "value (dec): " << token.value.integer << ", value (hex): " << (void*) token.value.integer << std::endl;
        }
        else if (EDL::Laxer_t::tk_symbol == token.id)
        {
            std::cout << "identifer: " << *token.value.symbol << std::endl;
            delete token.value.symbol;
        }
        else if (EDL::Laxer_t::tk_string == token.id)
        {
            std::cout << "string: " << *token.value.string << std::endl;
            delete token.value.string;
        }
        else
        {
            std::cerr << "invalid type: " << token.id << std::endl;
        }

    } while (token.id != EDL::Laxer_t::eof);

    return 0;
}
