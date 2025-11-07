#pragma once

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iterator>
#include <exception>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace regex {

    inline constexpr std::bitset<256> ascii_printable_chars = [] {
        std::bitset<256> bs;
        for (int i = ' '; i <= '~'; ++i) {
            bs.set(i);
        }
        return bs;
    }();

    class nfa {
       public:
        class state {
           public:
            using id_t         = std::uint32_t;
            using transition_t = std::vector<id_t>;
            using transition_map_t =
                std::array<transition_t, std::numeric_limits<unsigned char>::max() + 1>;

           private:
            transition_t epsilon_transitions;
            transition_map_t char_transitions;

           public:
            inline void add_transition(char input, id_t to)
            {
                this->char_transitions[input].push_back(to);
            }

            inline void add_epsilon_transition(id_t to)
            {
                this->epsilon_transitions.push_back(to);
            }

            // 获取转换映射的常量引用，用于复制NFA结构
            const transition_map_t& get_transition_map(void) const noexcept
            {
                return this->char_transitions;
            }

            const transition_t& get_transition(char input) const
            {
                return this->char_transitions[input];
            }

            const transition_t& get_epsilon_transition(void) const
            {
                return this->epsilon_transitions;
            }
        };

        using charset_t = std::bitset<256>;
        using states_t  = std::vector<state>;

       private:
        states_t states;
        state::id_t start, final;

       protected:
        explicit nfa(void)
        {
            this->set_start(this->add_state());
            this->set_final(this->add_state());
        }

        state::id_t add_state(void)
        {
            this->states.emplace_back();
            return this->states.size() - 1;
        }

        void check_state_valid(state::id_t state_n) const
        {
            if (state_n >= this->states.size()) {
                throw std::out_of_range(std::format("state_n({}) >= states.size({})",
                                                    state_n, this->states.size()));
            }
        }

        void set_start(state::id_t start)
        {
            this->check_state_valid(start);

            this->start = start;
        }

        void set_final(state::id_t final)
        {
            this->check_state_valid(final);

            this->final = final;
        }

        void add_transition(state::id_t state, char input, state::id_t to)
        {
            this->states.at(state).add_transition(input, to);
        }

        // 将charset中的字符添加为从start到final的转换
        void add_transition(const charset_t& chars, bool is_negated = false);

        // 添加从state到to的epsilon转换
        void add_epsilon_transition(state::id_t state, state::id_t to)
        {
            this->states.at(state).add_epsilon_transition(to);
        }

        // 内部辅助方法：将另一个NFA的状态和转换合并到当前NFA中
        // 返回偏移量，用于调整传入NFA的状态ID
        std::size_t merge_nfa_states(const nfa& other_nfa);

        // 连接当前NFA与传入的NFA
        // regexp: 当前NFA + next
        // 将传入的NFA连接到当前NFA之后
        void connect_with(const nfa& next);

        // 与传入的NFA进行选择操作
        // regexp: 当前NFA|other
        // 将传入的NFA与当前NFA进行选择操作（|）
        void select_with(const nfa& other);

       public:
        friend nfa build_nfa(std::string_view exp);

        class regex_error: public std::runtime_error {
           public:
            explicit regex_error(const std::string& msg): std::runtime_error(msg)
            {
            }
        };

        state::id_t get_start(void) const noexcept
        {
            return this->start;
        }

        state::id_t get_final(void) const noexcept
        {
            return this->final;
        }

        const state& get_state(state::id_t state) const noexcept
        {
            return this->states.at(state);
        }

        const states_t& get_states(void) const noexcept
        {
            return this->states;
        }

        // 合并两个NFA，实现选择操作（类似 | 操作符）
        nfa operator|(const nfa& other) const;

       private:
        // NFA builder类，包含所有构造相关的静态方法
        class builder {
           public:
            using charset_t = nfa::charset_t;

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

            // 从正则表达式创建NFA
            static nfa build(std::string_view exp);
        };
    };

} // namespace regex

// 为 nfa 类提供 std::format 支持
template<>
struct std::formatter<regex::nfa>
{
    std::string direction;

    constexpr auto parse(std::format_parse_context& ctx)
    {
        auto it = ctx.begin();

        while (*it != '}') {
            this->direction.push_back(*it++);
        }

        // 检查方向字符串是否有效
        if (this->direction != "LR" and this->direction != "TB"
            and this->direction != "RL" and not this->direction.empty()) {
            return ctx.end();
        }

        return it;
    }

    auto format(const regex::nfa& nfa, std::format_context& ctx) const
    {
        std::string result = "```mermaid\n";

        result += "stateDiagram-v2\n";

        if (not this->direction.empty()) {
            result += std::format("direction {}\n", this->direction);
        }

        result += "\n";

        auto& states = nfa.get_states();

        auto get_state_str = [&nfa](regex::nfa::state::id_t id) {
            if (id == nfa.get_start() || id == nfa.get_final()) {
                return std::string("[*]");
            } else {
                return std::format("state_{}", id);
            }
        };

        for (auto current_state_id = 0; current_state_id < states.size();
             current_state_id++) {
            if (current_state_id == nfa.get_final()) {
                continue;
            }

            const regex::nfa::state& state = states.at(current_state_id);
            std::string from               = get_state_str(current_state_id);

            // handle epsilon transitions
            for (auto& to_state : state.get_epsilon_transition()) {
                std::string to = get_state_str(to_state);

                result += std::format("{} --> {}\n", from, to);
            }

            // handle char transitions
            auto trans_map = state.get_transition_map();
            // 遍历所有可能的输入
            for (auto input = 0; input < trans_map.size(); input++) {
                auto& trans = trans_map.at(input);

                // 对输入字符, 遍历可能的目标状态
                for (auto& to_state : trans) {
                    std::string to = get_state_str(to_state);

                    result += std::format("{} --> {} : {}\n", from, to,
                                          static_cast<char>(input));
                }
            }
        }

        result += "```";

        return std::format_to(ctx.out(), "{}", result);
    }
};
