#pragma once

#include <cstdint>
#include <bitset>
#include <set>

namespace regex {
    using id_t      = std::uint32_t;
    using charset_t = std::bitset<256>;
    using closure_t = std::set<id_t>;
} // namespace regex
