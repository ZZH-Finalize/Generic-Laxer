#include "regex.hpp"

int main(const int argc, const char **argv)
{
    regex::nfa nfa1 = regex::nfa::from("abc");
    regex::nfa nfa2 = regex::nfa::from("a|b");

    return 0;
}
