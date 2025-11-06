#include "nfa.hpp"
#include <algorithm> // for std::copy

namespace laxer {
    void nfa::add_nfa(const regex::nfa& input_nfa)
    {
        // 记录合并前的状态数量作为偏移量
        std::size_t offset = this->get_states().size();

        // 将input_nfa的所有状态添加到当前NFA中（先添加空状态）
        for (std::size_t i = 0; i < input_nfa.get_states().size(); i++) {
            // 添加一个空状态
            this->add_state();
        }

        // 现在复制转换，调整目标ID
        for (std::size_t i = 0; i < input_nfa.get_states().size(); i++) {
            const auto& input_state = input_nfa.get_state(i);
            auto& new_state = this->get_state(i + offset);

            // 复制字符转换，调整目标ID
            const auto& char_transitions = input_state.get_transition_map();
            for (int input_idx = 0; input_idx < char_transitions.size(); input_idx++) {
                for (auto target_id : char_transitions[input_idx]) {
                    // 调整目标ID，加上偏移量
                    new_state.add_transition(static_cast<char>(input_idx),
                                             target_id + offset);
                }
            }

            // 复制epsilon转换，调整目标ID
            const auto& epsilon_transitions = input_state.get_epsilon_transition();
            for (auto target_id : epsilon_transitions) {
                // 调整目标ID，加上偏移量
                new_state.add_epsilon_transition(target_id + offset);
            }
        }

        // 从起始状态创建epsilon转换到新添加NFA的起始状态
        this->add_start_transition(input_nfa.get_start() + offset);

        // 为input_nfa的终态创建accept_state_t并添加到accept_states中
        this->emplace_accept_state(input_nfa.get_final() + offset);
    }
} // namespace laxer
