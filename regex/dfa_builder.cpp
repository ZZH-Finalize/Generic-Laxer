#include "dfa.hpp"

namespace regex {

    dfa::builder::state_set_t dfa::builder::epsilon_closure(const state_set_t& states_set,
                                                            const nfa& input_nfa)
    {
        state_set_t closure = states_set;
        std::vector<bool> in_closure(input_nfa.get_states().size(), false);

        // 标记初始状态在闭包中
        for (auto state_id : states_set) {
            if (state_id < in_closure.size()) {
                in_closure[state_id] = true;
            }
        }

        // 使用队列进行BFS遍历，避免无限循环
        std::queue<nfa::state::id_t> work_queue;
        for (auto state_id : states_set) {
            work_queue.push(state_id);
        }

        while (!work_queue.empty()) {
            nfa::state::id_t current_state = work_queue.front();
            work_queue.pop();

            if (current_state >= input_nfa.get_states().size()) continue;

            const auto& state           = input_nfa.get_state(current_state);
            const auto& epsilon_targets = state.get_epsilon_transition();

            for (auto epsilon_target : epsilon_targets) {
                if (epsilon_target < in_closure.size()
                    and not in_closure[epsilon_target]) {
                    in_closure[epsilon_target] = true;

                    closure.insert(epsilon_target);
                    work_queue.push(epsilon_target);
                }
            }
        }
        return closure;
    }

    dfa::builder::state_set_t dfa::builder::move(const state_set_t& states_set,
                                                 char input, const nfa& input_nfa)
    {
        state_set_t result;
        for (auto state_id : states_set) {
            if (state_id >= input_nfa.get_states().size()) continue;

            const auto& state          = input_nfa.get_state(state_id);
            const auto& transition_map = state.get_transition_map();

            if (input >= 0) {
                for (auto target : transition_map[static_cast<unsigned char>(input)]) {
                    result.insert(target);
                }
            }
        }

        return result;
    }

    dfa dfa::builder::build(const nfa& input_nfa)
    {
        dfa result_dfa;

        if (input_nfa.get_states().empty()) {
            // 如果NFA没有状态，创建一个空的DFA
            result_dfa.start_state = result_dfa.add_state({});
            return result_dfa;
        }

        // 计算初始状态的epsilon闭包
        state_set_t initial_closure =
            builder::epsilon_closure(input_nfa.get_start(), input_nfa);
        std::map<state_set_t, dfa::state::id_t> state_map; // 映射NFA状态集到DFA状态ID
        std::vector<state_set_t> unmarked;                 // 未标记的DFA状态

        // 创建初始DFA状态
        bool is_final          = initial_closure.contains(input_nfa.get_final());
        result_dfa.start_state = result_dfa.add_state(initial_closure, is_final);

        state_map[initial_closure] = 0;

        unmarked.push_back(initial_closure);

        // 子集构造算法
        while (not unmarked.empty()) {
            state_set_t current_set = unmarked.back();
            unmarked.pop_back();

            dfa::state::id_t current_id = state_map[current_set];

            // 尝试所有可能的输入字符
            std::set<char> input_chars;
            for (auto nfa_state_id : current_set) {
                if (nfa_state_id >= input_nfa.get_states().size()) continue;

                const auto& nfa_state   = input_nfa.get_state(nfa_state_id);
                const auto& transitions = nfa_state.get_transition_map();
                for (int input_idx = 0; input_idx < transitions.size(); input_idx++) {
                    if (not transitions[input_idx].empty()) {
                        input_chars.insert(static_cast<char>(input_idx));
                    }
                }
            }

            // 对每个输入字符计算下一个状态
            for (char input_char : input_chars) {
                state_set_t next_set = builder::epsilon_closure(
                    builder::move(current_set, input_char, input_nfa), input_nfa);
                if (next_set.empty()) continue;

                dfa::state::id_t next_id;
                auto it = state_map.find(next_set);
                if (it == state_map.end()) {
                    // 创建新的DFA状态

                    next_id = result_dfa.add_state(
                        next_set, next_set.contains(input_nfa.get_final()));
                    state_map[next_set] = next_id;
                    unmarked.push_back(next_set);
                } else {
                    next_id = it->second;
                }

                // 添加转换 - 修复：每次都重新获取引用以避免引用失效
                result_dfa.states[current_id].set_transition(input_char, next_id);
            }
        }

        // 记录最终状态
        for (dfa::state::id_t i = 0; i < result_dfa.states.size(); i++) {
            if (result_dfa.states[i].is_final()) {
                result_dfa.final_states.insert(i);
            }
        }

        return result_dfa;
    }

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

} // namespace regex
