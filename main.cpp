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

        if (EDL::Token_t::invalid == token) {
            laxer.skip_to_next_line();
        }

        std::cout << std::format("{}", token) << std::endl;

    } while (token != EDL::Token_t::eof);

    test_file.close();

    return 0;
}
