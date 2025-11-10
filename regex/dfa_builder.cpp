#include "dfa.hpp"
#include "final_state.hpp"
#include "nfa.hpp"

namespace regex {

#if 1
    dfa dfa::builder::minimize(const dfa& input_dfa)
    {
        return input_dfa;
    }
#else
    dfa dfa::builder::minimize(const dfa& input_dfa)
    {
        if (input_dfa.states.empty()) {
            return input_dfa;
        }

        // 初始化等价类划分：将状态分为最终状态和非最终状态
        std::vector<std::set<dfa::state::id_t>> partition;
        std::set<dfa::state::id_t> final_state_set = input_dfa.final_states;

        std::set<dfa::state::id_t> non_final_states;
        for (dfa::state::id_t i = 0; i < input_dfa.states.size(); i++) {
            if (final_state_set.find(i) == final_state_set.end()) {
                non_final_states.insert(i);
            }
        }

        // 添加最终状态集合（如果非空）
        if (!final_state_set.empty()) {
            partition.push_back(final_state_set);
        }

        // 添加非最终状态集合（如果非空）
        if (!non_final_states.empty()) {
            partition.push_back(non_final_states);
        }

        // 获取DFA中实际使用的字符
        std::set<char> alphabet;
        for (const auto& state : input_dfa.states) {
            const auto& transition_map = state.get_transition_map();
            for (int i = 0; i < transition_map.size(); ++i) {
                if (transition_map[i] != std::numeric_limits<dfa::state::id_t>::max()) {
                    alphabet.insert(static_cast<char>(i));
                }
            }
        }

        bool changed = true;
        while (changed) {
            changed = false;
            std::vector<std::set<dfa::state::id_t>> new_partition;

            for (const auto& block : partition) {
                if (block.size() <= 1) {
                    // 单元素块无法进一步分割
                    new_partition.push_back(block);
                    continue;
                }

                // 尝试按输入字符分割当前块
                bool split_occurred = false;
                std::vector<std::set<dfa::state::id_t>> temp_partition;
                // 初始化第一个子集，包含block的第一个元素
                std::set<dfa::state::id_t> initial_subset;
                initial_subset.insert(*block.begin());
                temp_partition.push_back(initial_subset);

                for (auto it = std::next(block.begin()); it != block.end(); ++it) {
                    dfa::state::id_t current_state = *it;
                    bool placed                    = false;

                    for (auto& temp_block : temp_partition) {
                        dfa::state::id_t representative =
                            *temp_block.begin(); // 选一个代表状态
                        bool equivalent = true;

                        // 检查两个状态是否等价（对于所有输入字符，转换到的等价类相同）
                        for (char c : alphabet) {
                            dfa::state::id_t current_next =
                                input_dfa.states[current_state].get_transition(c);
                            dfa::state::id_t rep_next =
                                input_dfa.states[representative].get_transition(c);

                            // 如果一个状态有转换而另一个没有，则不等价
                            if ((current_next
                                 == std::numeric_limits<dfa::state::id_t>::max())
                                != (rep_next
                                    == std::numeric_limits<dfa::state::id_t>::max())) {
                                equivalent = false;
                                break;
                            }

                            if (current_next
                                    != std::numeric_limits<dfa::state::id_t>::max()
                                && rep_next
                                       != std::numeric_limits<dfa::state::id_t>::max()) {
                                // 找到这两个状态所属的等价类
                                int current_class = -1, rep_class = -1;
                                for (int i = 0; i < partition.size(); ++i) {
                                    if (partition[i].count(current_next))
                                        current_class = i;
                                    if (partition[i].count(rep_next)) rep_class = i;
                                }

                                if (current_class != rep_class) {
                                    equivalent = false;
                                    break;
                                }
                            }
                        }

                        if (equivalent) {
                            temp_block.insert(current_state);
                            placed = true;
                            break;
                        }
                    }

                    if (!placed) {
                        std::set<dfa::state::id_t> new_subset;
                        new_subset.insert(current_state);
                        temp_partition.push_back(new_subset);
                    }
                }

                if (temp_partition.size() > 1) {
                    changed        = true;
                    split_occurred = true;
                    for (const auto& new_block : temp_partition) {
                        if (!new_block.empty()) {
                            new_partition.push_back(new_block);
                        }
                    }
                } else {
                    new_partition.push_back(block);
                }
            }

            if (changed) {
                partition = new_partition;
            }
        }

        // 创建最小化DFA
        dfa minimized_dfa;

        // 创建映射：原状态ID -> 最小化后状态ID
        std::map<dfa::state::id_t, dfa::state::id_t> state_mapping;

        for (dfa::state::id_t i = 0; i < partition.size(); i++) {
            for (auto state_id : partition[i]) {
                state_mapping[state_id] = i;
            }
        }

        // 构建最小化的DFA状态
        for (dfa::state::id_t i = 0; i < partition.size(); i++) {
            // 从等价类中任选一个状态来决定新状态的属性
            dfa::state::id_t representative = *partition[i].begin();
            bool is_final                   = input_dfa.states[representative].is_final();

            // 创建新状态并添加到最小化DFA中
            dfa::state::state_set_t dummy_set; // 构造函数需要，但对最小化DFA不重要
            dfa::state::id_t new_state_id = minimized_dfa.add_state(dummy_set, is_final);

            // 设置转换
            for (char c : alphabet) {
                dfa::state::id_t old_next =
                    input_dfa.states[representative].get_transition(c);
                if (old_next != std::numeric_limits<dfa::state::id_t>::max()) {
                    dfa::state::id_t new_next = state_mapping[old_next];
                    minimized_dfa.states[new_state_id].set_transition(c, new_next);
                }
            }
        }

        // 设置起始状态
        minimized_dfa.start_state = state_mapping[input_dfa.start_state];

        // 设置最终状态列表
        for (auto final_id : input_dfa.final_states) {
            dfa::state::id_t new_final_id = state_mapping[final_id];
            minimized_dfa.final_states.insert(new_final_id);
        }

        return minimized_dfa;
    }
#endif

} // namespace regex
