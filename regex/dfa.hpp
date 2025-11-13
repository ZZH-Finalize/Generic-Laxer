#pragma once

#include <format>
#include <iterator>
#include <optional>
#include <set>
#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include <map>

#include "basic_fa.hpp"

namespace regex {
    class __dfa_state: public basic_state<std::uint32_t> {
       private:
        // 对应NFA状态集合
        closure nfa_states;

        // 是否为最终状态
        bool final;

       public:
        // 构造函数初始化转换表为无效值
        explicit __dfa_state(const closure& nfa_states = {}, bool is_final = false)
            : nfa_states(nfa_states), final(is_final)
        {
            std::fill(this->transition_map.begin(), this->transition_map.end(),
                      std::numeric_limits<id_t>::max());
        }

        inline void set_final(bool is_final)
        {
            this->final = is_final;
        }

        inline bool is_final(void) const noexcept
        {
            return this->final;
        }

        inline const closure& get_closure(void) const
        {
            return this->nfa_states;
        }
    };

    class dfa: public basic_fa<__dfa_state> {
       protected:
        std::set<id_t> final;

        inline const auto& get_final(void) const noexcept
        {
            return this->final;
        }

        inline void add_final(const id_t& state)
        {
            this->final.insert(state);
        }

       public:
        friend class builder;
        friend struct std::formatter<dfa>;

        // 匹配算法：检查字符串是否与DFA匹配
        std::optional<id_t> match(std::string_view str) const;

        // 检查是否匹配空字符串
        bool match_empty() const;

        // 查找匹配：检查输入字符串中是否存在匹配模式的子串
        // 返回匹配的起始位置，-1表示未找到匹配
        int find_match(std::string_view str) const;
    };

} // namespace regex
