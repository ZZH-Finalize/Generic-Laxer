#include <string_view>
#include <map>
#include <queue>

#include "nfa.hpp"

namespace regex {
    // builder类，包含所有构造相关的静态方法
    class builder {
       public:
        using transition_map_t =
            std::array<id_t, std::tuple_size_v<nfa::state::transition_map_t>>;

        // 从正则表达式创建NFA
        static nfa build(std::string_view exp);

        template<typename NFA>
        requires is_nfa<NFA>
        static NFA::dfa build(const NFA& input_nfa)
        {
            using final_state_id_t = typename NFA::dfa::final_state_id_t;
            typename NFA::dfa result_dfa;

            if (input_nfa.get_states().empty()) {
                // 如果NFA没有状态，创建一个空的DFA
                closure_t empty_closure;
                result_dfa.set_start(result_dfa.add_state(empty_closure, false));
                return result_dfa;
            }

            // 计算初始状态的epsilon闭包
            closure_t initial_closure =
                builder::epsilon_closure(input_nfa.get_start(), input_nfa);
            std::map<closure_t, id_t> state_map; // 映射NFA状态集到DFA状态ID
            std::vector<closure_t> unmarked;     // 未标记的DFA状态

            // 创建初始DFA状态
            bool is_final = input_nfa.find_final(initial_closure).has_value();
            result_dfa.set_start(result_dfa.add_state(initial_closure, is_final));

            state_map[initial_closure] = result_dfa.get_start();

            unmarked.push_back(initial_closure);

            // 子集构造算法
            while (not unmarked.empty()) {
                closure_t current_set = unmarked.back();
                unmarked.pop_back();

                id_t current_id = state_map[current_set];

                // 尝试所有可能的输入字符
                std::set<char> input_chars;
                for (auto nfa_state_id : current_set) {
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
                    closure_t next_set = builder::epsilon_closure(
                        builder::move(current_set, input_char, input_nfa), input_nfa);
                    if (next_set.empty()) continue;

                    id_t next_id;
                    auto it = state_map.find(next_set);
                    if (it == state_map.end()) {
                        // 创建新的DFA状态
                        next_id = result_dfa.add_state(
                            next_set, input_nfa.find_final(next_set).has_value());
                        state_map[next_set] = next_id;
                        unmarked.push_back(next_set);
                    } else {
                        next_id = it->second;
                    }

                    // 添加转换
                    result_dfa.set_transition(current_id, input_char, next_id);
                }
            }

            // 记录最终状态
            for (id_t i = 0; i < result_dfa.get_states().size(); i++) {
                auto state = result_dfa.get_state(i);
                if (state.is_final()) {
                    const auto& closure = state.get_closure();

                    final_state_id_t final_state(i);

                    if constexpr (has_metadata<final_state_id_t>) {
                        auto final = input_nfa.find_final(closure);

                        if (final.has_value()) {
                            final_state.copy_metadata(final.value());
                        }
                    }

                    result_dfa.add_final(final_state);
                }
            }

            return result_dfa;
        }

        // DFA最小化算法 - 使用Hopcroft算法
        // static dfa minimize(const dfa& input_dfa);

       private:
        // 计算epsilon闭包
        template<typename NFA>
        requires is_nfa<NFA>
        static closure_t epsilon_closure(const closure_t& states_set,
                                         const NFA& input_nfa)
        {
            closure_t closure = states_set;

            // 使用队列进行BFS遍历，避免无限循环
            std::queue<id_t> work_queue;
            for (auto state_id : states_set) {
                work_queue.push(state_id);
            }

            while (not work_queue.empty()) {
                id_t current_state = work_queue.front();
                work_queue.pop();

                const auto& state           = input_nfa.get_state(current_state);
                const auto& epsilon_targets = state.get_epsilon_transition();

                for (auto epsilon_target : epsilon_targets) {
                    closure.insert(epsilon_target);
                    work_queue.push(epsilon_target);
                }
            }

            return closure;
        }

        template<typename NFA>
        requires is_nfa<NFA>
        static closure_t epsilon_closure(id_t state_id, const NFA& input_nfa)
        {
            closure_t single_set = {state_id};
            return builder::epsilon_closure(single_set, input_nfa);
        }

        template<typename NFA>
        requires is_nfa<NFA>
        static closure_t move(const closure_t& states_set, char input,
                              const NFA& input_nfa)
        {
            closure_t result;

            for (auto state_id : states_set) {
                const auto& state          = input_nfa.get_state(state_id);
                const auto& transition_map = state.get_transition_map();

                for (auto target : transition_map[static_cast<unsigned char>(input)]) {
                    result.insert(target);
                }
            }

            return result;
        }

        // 处理转义字符, 当读到\的时候, 把下一字符传进来, 将返回对应的转义字符
        static char handle_escape(char ch);

        // 内部解析辅助函数
        // 解析单个字符（包括转义字符）并推进指针
        static nfa parse_char(std::string_view& exp);

        // 解析通配符.并推进指针
        static nfa parse_wildcard(std::string_view& exp);

        // 解析字符集[...]并推进指针
        static nfa parse_charset(std::string_view& exp);

        // 解析数字相关字符集(\d, \D)并推进指针
        static nfa parse_digit(std::string_view& exp);

        // 解析单词相关字符集(\w, \W)并推进指针
        static nfa parse_word(std::string_view& exp);

        // 解析空白字符集(\s, \S)并推进指针
        static nfa parse_space(std::string_view& exp);

        // 递归解析表达式的主要函数
        static nfa parse_expression(std::string_view& exp);

        // 解析序列（连接操作）
        static nfa parse_sequence(std::string_view& exp);

        // 解析单个项（可能包含量词）
        static nfa parse_term(std::string_view& exp);
    };

} // namespace regex
