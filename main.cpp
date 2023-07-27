#include <iostream>
#include <fstream>
#include "laxer.h"

int main(const int argc, const char** argv)
{
    std::ifstream test_file("test_cases/dec.edl");

    if (false == test_file.is_open())
    {
        std::cout << "file open fail" << std::endl;
        return -1;
    }

    EDL::Laxer_t laxer(test_file);

    EDL::Laxer_t::token_t token = laxer.next_token();

    std::cout << "token_type:" << token.id << ", value:" << token.value.integer << std::endl;


    return 0;
}
