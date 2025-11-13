#include "nfa.hpp"

namespace regex {
    std::size_t nfa::merge_states(const nfa& other_nfa)
    {
        // 保存当前NFA的原始状态数量作为偏移量
        std::size_t offset = this->states.size();

        // 添加other_nfa的所有状态到当前NFA中
        for (const auto& s : other_nfa.states) {
            this->add_state();
        }

        // 复制other_nfa的转换，但需要调整ID偏移
        for (id_t i = 0; i < other_nfa.states.size(); i++) {
            const auto& other_state          = other_nfa.states[i];
            const auto& other_transition_map = other_state.get_transition_map();

            // 复制字符转换
            for (int input_idx = 0; input_idx < other_transition_map.size();
                 input_idx++) {
                if (not other_transition_map[input_idx].empty()) {
                    char input_char = static_cast<char>(input_idx);

                    for (auto id : other_transition_map[input_idx]) {
                        this->add_transition(i + offset, input_char, id + offset);
                    }
                }
            }

            // 处理epsilon转换
            for (auto id : other_state.get_epsilon_transition()) {
                this->add_epsilon_transition(i + offset, id + offset);
            }
        }

        return offset;
    }

    void nfa::connect_with(const nfa& next)
    {
        // 使用公共方法合并next的状态和转换
        std::size_t offset = this->merge_states(next);

        // 连接当前NFA的最终状态和next的起始状态
        this->add_epsilon_transition(this->get_final(), next.start + offset);

        // 更新当前NFA的最终状态为next的最终状态（加上偏移）
        this->set_final(next.get_final() + offset);
    }

    void nfa::select_with(const nfa& other)
    {
        // 保存当前NFA的原始起始和最终状态
        id_t original_start = this->get_start();
        id_t original_final = this->get_final();

        // 使用公共方法合并other的状态和转换
        std::size_t offset = this->merge_states(other);

        // 创建新的起始状态和最终状态
        id_t new_start = this->add_state();
        id_t new_final = this->add_state();

        // 从新的起始状态到原来的两个NFA的起始状态添加epsilon转换
        this->add_epsilon_transition(new_start, original_start);
        this->add_epsilon_transition(new_start, other.start + offset);

        // 从原来的两个NFA的最终状态到新的最终状态添加epsilon转换
        this->add_epsilon_transition(original_final, new_final);
        this->add_epsilon_transition(other.get_final() + offset, new_final);

        // 更新当前NFA的起始和最终状态
        this->set_start(new_start);
        this->set_final(new_final);
    }

    nfa nfa::operator|(const nfa& other) const
    {
        nfa result;

        // 保存当前NFA的原始起始和最终状态
        id_t original_start = this->get_start();
        id_t original_final = this->get_final();

        // 使用公共方法合并other的状态和转换
        std::size_t offset       = result.merge_states(*this);
        std::size_t other_offset = result.merge_states(other);

        // 创建新的起始状态和最终状态
        id_t new_start = result.add_state();
        id_t new_final = result.add_state();

        // 从新的起始状态到原来的两个NFA的起始状态添加epsilon转换
        result.add_epsilon_transition(new_start, original_start + offset);
        result.add_epsilon_transition(new_start, other.start + other_offset);

        // 从原来的两个NFA的最终状态到新的最终状态添加epsilon转换
        result.add_epsilon_transition(original_final + offset, new_final);
        result.add_epsilon_transition(other.get_final() + other_offset, new_final);

        // 更新当前NFA的起始和最终状态
        result.set_start(new_start);
        result.set_final(new_final);

        return result;
    }
} // namespace regex
