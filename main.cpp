#include <format>
#include <iostream>
#include <fstream>
#include <string>

#include "nfa.hpp"

int main(const int argc, const char** argv)
{
    std::ifstream test_file(argv[1]);

    if (false == test_file.is_open()) {
        std::cout << "file open fail" << std::endl;
        return -1;
    }



    test_file.close();

    return 0;
}
