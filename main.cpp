#include <format>
#include <iostream>
#include <fstream>
#include <string>
#include "laxer.h"

int main(const int argc, const char** argv)
{
    std::ifstream test_file(argv[1]);

    if (false == test_file.is_open()) {
        std::cout << "file open fail" << std::endl;
        return -1;
    }

    EDL::Laxer_t laxer(test_file, std::cout, false);
    EDL::Token_t token;

    do {
        token = std::move(laxer.next_token());

        switch (token) {
            case EDL::Token_t::invalid: {
                std::cout << "identifer error, skip to next line" << std::endl;
                laxer.skip_to_next_line();
            } break;

            case EDL::Token_t::eof: {
                std::cout << "achieve the end of the file" << std::endl;
            } break;

            case EDL::Token_t::tk_integer: {
                auto number = token.value<std::uint64_t>();

                std::cout << std::format("value (dec): {}, value (hex): {:#X}", number,
                                         number)
                          << std::endl;
            } break;

            case EDL::Token_t::tk_number: {
                auto number = token.value<double>();
                std::cout << "value: " << number << std::endl;
            } break;

            case EDL::Token_t::tk_symbol: {
                const auto& sym = token.value<std::string>();
                std::cout << "identifer: " << sym << std::endl;
            } break;

            case EDL::Token_t::tk_string: {
                const auto& str = token.value<std::string>();
                std::cout << "string: " << str << std::endl;
            } break;

            case ' ' ... '~': {
                std::cout << "ascii char: " << static_cast<char>(token) << std::endl;
            } break;

            default: {
                std::cerr << "invalid type: " << token << std::endl;
            } break;
        }

    } while (token != EDL::Token_t::eof);

    test_file.close();

    return 0;
}
