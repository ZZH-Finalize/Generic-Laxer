#pragma once

#include <set>

#include "regex_typedef.hpp"
#include "basic_dfa.hpp"

namespace regex {

    using dfa = basic_dfa<std::set<id_t>>;

} // namespace regex
