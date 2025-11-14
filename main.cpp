#include <any>
#include <format>
#include <iostream>
#include <fstream>
#include <optional>
#include <string>

#include "dfa.hpp"
#include "regex/regex.hpp"
#include "nfa.hpp"

int main(const int argc, const char** argv)
{
    regex::nfa rule1 = regex::build_nfa("else");

    laxer::nfa nfa;

    nfa.add_nfa(rule1);

    laxer::dfa dfa = regex::to_dfa(nfa);

    
    return 0;
}
