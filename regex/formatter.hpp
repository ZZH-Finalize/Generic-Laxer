#pragma once

#include "nfa.hpp"
#include <format>

// 为 nfa 类提供 std::format 支持
template<>
struct std::formatter<regex::nfa>
{
    constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin(); // 简单格式，无需解析额外格式说明符
    }

    auto format(const regex::nfa& nfa, std::format_context& ctx) const
    {
        std::string nfa_str = nfa.to_string();
        return std::format_to(ctx.out(), "{}", nfa_str);
    }
};
