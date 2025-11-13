#include "dfa.hpp"
#include <optional>
#include "final_state.hpp"

namespace regex {

    std::optional<final_state> dfa::match(std::string_view str) const
    {
        if (this->states.empty()) {
            return std::nullopt; // 没有状态，无法匹配
        }

        state::id_t current_state = this->get_start();

        for (char c : str) {
            // 边界检查：确保current_state在有效范围内
            if (current_state >= this->states.size()) {
                return std::nullopt; // 状态越界，匹配失败
            }

            const auto& current_dfa_state = this->states[current_state];
            // 检查转换是否有效（使用特殊值表示无效转换）
            state::id_t next_state = current_dfa_state.get_transition(c);
            if (next_state == dfa::invalid_state) {
                // 没有对应的转换，匹配失败
                return std::nullopt;
            }

            current_state = next_state;
        }

        const auto& find_state = this->get_final().find(final_state(current_state));

        // 检查最终状态是否为接受状态
        if (find_state != this->get_final().end()) {
            return *find_state;
        }

        return std::nullopt;
    }

    bool dfa::match_empty() const
    {
        if (this->states.empty()) {
            return false;
        }
        // 边界检查：确保this->get_start()在有效范围内
        if (this->get_start() >= this->states.size()) {
            return false;
        }
        return this->get_final().contains(final_state(this->get_start()));
    }

    int dfa::find_match(std::string_view str) const
    {
        if (this->states.empty()) {
            return -1; // 没有状态，无法匹配
        }

        // 尝试从每个位置开始匹配
        for (size_t start_pos = 0; start_pos < str.length(); ++start_pos) {
            state::id_t current_state = this->get_start();

            // 从当前位置开始尝试匹配
            for (size_t i = start_pos; i < str.length(); ++i) {
                char c = str[i];

                // 边界检查：确保current_state在有效范围内
                if (current_state >= this->states.size()) {
                    break; // 状态越界，此路径匹配失败
                }

                const auto& current_dfa_state = this->states[current_state];
                // 检查转换是否有效（使用特殊值表示无效转换）
                state::id_t next_state = current_dfa_state.get_transition(c);
                if (next_state == dfa::invalid_state) {
                    // 没有对应的转换，匹配失败
                    break;
                }

                current_state = next_state;

                // 检查当前位置是否为接受状态
                if (this->get_final().contains(final_state(current_state))) {
                    // 找到匹配，返回起始位置
                    return static_cast<int>(start_pos);
                }
            }
        }

        // 没有找到匹配
        return -1;
    }

} // namespace regex
