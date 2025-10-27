#pragma once

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iterator>
#include <map>
#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

template<typename T>
concept id_type_c = std::is_integral_v<T>;

namespace regex {

    inline constexpr std::bitset<256> ascii_printable_chars = [] {
        std::bitset<256> bs;
        for (int i = ' '; i <= '~'; ++i) {
            bs.set(i);
        }
        return bs;
    }();

    class state {
       public:
        using id_type        = std::uint32_t;
        using transition_map = std::unordered_map<char, std::vector<id_type>>;

       private:
        inline static state::id_type id_counter = 1;
        id_type __id;
        transition_map transitions;

       public:
        state(): __id(this->id_counter++)
        {
        }

        void add_transition(char input, id_type to)
        {
            this->transitions[input].push_back(to);
        }

        void add_epsilon_transition(id_type to)
        {
            this->transitions['\0'].push_back(to); // 使用空字符表示epsilon转换
        }

        // 获取转换映射的常量引用，用于复制NFA结构
        const transition_map& get_transitions() const
        {
            return this->transitions;
        }
    };

    class nfa {
       private:
        using charset_t = std::bitset<256>;
    
        std::vector<state> states;
        state::id_type start, final;

        explicit nfa()
        {
            this->set_start(this->add_state());
            this->set_final(this->add_state());
        }

        state::id_type add_state(void)
        {
            this->states.emplace_back();
            return this->states.size() - 1;
        }

        void check_state_valid(state::id_type state_n) const
        {
            if (state_n >= this->states.size()) {
                throw std::out_of_range(std::format("state_n({}) >= states.size({})",
                                                    state_n, this->states.size()));
            }
        }

        void set_start(state::id_type start)
        {
            this->check_state_valid(start);

            this->start = start;
        }

        void set_final(state::id_type final)
        {
            this->check_state_valid(final);

            this->final = final;
        }

        state::id_type get_start(void) const noexcept
        {
            return this->start;
        }

        state::id_type get_final(void) const noexcept
        {
            return this->final;
        }

        state& get_state(state::id_type state)
        {
            return this->states.at(state);
        }

        void add_transition(state::id_type state, char input, state::id_type to)
        {
            this->states.at(state).add_transition(input, to);
        }

        // 将charset中的字符添加为从start到final的转换
        void add_transition(const charset_t& chars, bool is_negated = false)
        {
            charset_t final_charset = is_negated ? (regex::ascii_printable_chars & ~chars) : chars;
            for (std::size_t i = 0; i < 256; i++) {
                if (final_charset.test(i)) {
                    this->add_transition(this->get_start(), static_cast<char>(i),
                                         this->get_final());
                }
            }
        }

        void add_epsilon_transition(state::id_type state, state::id_type to)
        {
            this->states.at(state).add_epsilon_transition(to);
        }

        // 处理转义字符, 当读到\的时候, 把下一字符传进来, 将返回对应的转义字符
        static char handle_escape(char ch)
        {
            switch (ch) {
                case 'n': return '\n';
                case 'r': return '\r';
                case 't': return '\t';

                case 'a': return '\a';
                case 'b': return '\b';
                case 'f': return '\f';
                case 'v': return '\v';

                default: return ch;
            }
        }

        // 内部解析辅助函数
       private:
        // 解析单个字符（包括转义字符）并推进指针
        static nfa parse_char(std::string_view& exp)
        {
            if (exp.empty()) {
                throw nfa::regex_error("Unexpected end of expression");
            }

            char ch = exp[0];
            if (ch == '\\') {
                if (exp.length() < 2) {
                    throw nfa::regex_error("Unexpected end of expression after escape");
                }

                ch = nfa::handle_escape(exp[1]);
                exp.remove_prefix(2); // 跳过转义字符
            } else {
                exp.remove_prefix(1); // 跳过普通字符
            }

            nfa result;
            result.add_transition(result.get_start(), ch, result.get_final());
            return result;
        }

        // 解析通配符.并推进指针
        static nfa parse_wildcard(std::string_view& exp)
        {
            if (exp.empty() || exp[0] != '.') {
                throw nfa::regex_error("Expected wildcard character '.'");
            }
            exp.remove_prefix(1); // 跳过'.'字符

            nfa result;
            result.add_transition(regex::ascii_printable_chars);

            return result;
        }

        // 解析字符集[...]并推进指针
        static nfa parse_charset(std::string_view& exp)
        {
            if (exp.empty() || exp[0] != '[') {
                throw nfa::regex_error("Expected character set starting with '['");
            }

            nfa result;
            nfa::charset_t chars;

            const auto exp_orig = exp;

            bool is_negated = '^' == exp[1];
            bool is_escape  = false;
            bool has_end    = false;

            exp.remove_prefix(is_negated ? 2 : 1);

            for (std::size_t i = 0; i < exp.length(); i++) {
                char current = is_escape ? nfa::handle_escape(exp[i]) : exp[i];

                if (current == ']') {
                    has_end = true;
                    exp.remove_prefix(i + 1); // 跳过当前字符及之前的所有字符
                    break;
                } else if (current == '\\' and not is_escape) {
                    is_escape = true;
                    continue;
                }

                is_escape = false;

                // 处理range
                if (((i + 2) < exp.length()) and (exp[i + 1] == '-')
                    and (exp[i + 2] != ']')) {
                    char range_begin = current;
                    char range_end   = exp[i + 2];

                    // 普通的range需要跳过2字符(-b), 如果range_end是转义字符那就是3个
                    std::size_t offset = 2;

                    if (range_end == '\\') {
                        // range_end依然是转义字符, 且\后还有有效字符
                        if ((i + 3) < exp.length()) {
                            range_end = nfa::handle_escape(exp[i + 3]);
                            offset += 1;
                        } else {
                            throw nfa::regex_error(
                                std::format("invalid range in regexp: {}", exp_orig));
                        }
                    }

                    if (range_end < range_begin) {
                        throw nfa::regex_error(
                            std::format("begin({}) is greater than end({}) in range",
                                        range_begin, range_end));
                    }

                    for (char ch = range_begin; ch <= range_end; ch++) {
                        chars.set(static_cast<unsigned char>(ch));
                    }

                    i += offset;
                } else {
                    chars.set(static_cast<unsigned char>(current));
                }
            }

            if (not has_end) {
                throw nfa::regex_error(std::format("none-closed charset: {}", exp_orig));
            }

            // 为所有符合条件的字符添加转换规则
            result.add_transition(chars, is_negated);

            return result;
        }

        // 解析数字相关字符集(\d, \D)并推进指针
        static nfa parse_digit(std::string_view& exp)
        {
            if (exp.length() < 2 or exp[0] != '\\' or (exp[1] != 'd' and exp[1] != 'D')) {
                throw nfa::regex_error("Expected digit character class '\\d' or '\\D'");
            }

            bool is_negated = exp[1] == 'D';
            exp.remove_prefix(2); // 跳过'\d'或'\D'

            nfa result;
            nfa::charset_t chars;

            // 添加数字字符 0-9
            for (char ch = '0'; ch <= '9'; ch++) {
                chars.set(static_cast<unsigned char>(ch));
            }

            // 为所有符合条件的字符添加转换规则
            result.add_transition(chars, is_negated);

            return result;
        }

        // 解析单词相关字符集(\w, \W)并推进指针
        static nfa parse_word(std::string_view& exp)
        {
            if (exp.length() < 2 || exp[0] != '\\' || (exp[1] != 'w' && exp[1] != 'W')) {
                throw nfa::regex_error("Expected word character class '\\w' or '\\W'");
            }

            bool is_negated = exp[1] == 'W';
            exp.remove_prefix(2); // 跳过'\w'或'\W'

            nfa result;
            nfa::charset_t chars;

            // 添加字母 a-z, A-Z, 数字 0-9, 以及下划线 _
            for (char ch = 'a'; ch <= 'z'; ch++) {
                chars.set(static_cast<unsigned char>(ch));
            }

            for (char ch = 'A'; ch <= 'Z'; ch++) {
                chars.set(static_cast<unsigned char>(ch));
            }

            for (char ch = '0'; ch <= '9'; ch++) {
                chars.set(static_cast<unsigned char>(ch));
            }

            chars.set(static_cast<unsigned char>('_'));

            // 为所有符合条件的字符添加转换规则
            result.add_transition(chars, is_negated);

            return result;
        }

        // 解析空白字符集(\s, \S)并推进指针
        static nfa parse_space(std::string_view& exp)
        {
            if (exp.length() < 2 || exp[0] != '\\' || (exp[1] != 's' && exp[1] != 'S')) {
                throw nfa::regex_error("Expected space character class '\\s' or '\\S'");
            }

            bool is_negated = exp[1] == 'S';
            exp.remove_prefix(2); // 跳过'\s'或'\S'

            nfa result;
            nfa::charset_t chars;

            for (char ch : {' ', '\t', '\r', '\n', '\b', '\f', '\v'}) {
                chars.set(static_cast<unsigned char>(ch));
            }

            nfa::charset_t final_charset =
                is_negated ? (regex::ascii_printable_chars & ~chars) : chars;

            // 为所有符合条件的字符添加转换规则
            result.add_transition(final_charset);

            return result;
        }

        // 内部辅助方法：将另一个NFA的状态和转换合并到当前NFA中
        // 返回偏移量，用于调整传入NFA的状态ID
        std::size_t merge_nfa_states(const nfa& other_nfa)
        {
            // 保存当前NFA的原始状态数量作为偏移量
            std::size_t offset = this->states.size();

            // 添加other_nfa的所有状态到当前NFA中
            for (const auto& s : other_nfa.states) {
                this->add_state();
            }

            // 复制other_nfa的转换，但需要调整ID偏移
            for (state::id_type i = 0; i < other_nfa.states.size(); ++i) {
                const auto& other_state = other_nfa.states[i];
                for (const auto& [input, id_list] : other_state.get_transitions()) {
                    for (auto id : id_list) {
                        this->add_transition(i + offset, input, id + offset);
                    }
                }
            }

            return offset;
        }

        // 连接当前NFA与传入的NFA
        // regexp: 当前NFA + next
        // 将传入的NFA连接到当前NFA之后
        void connect_with(const nfa& next)
        {
            // 使用公共方法合并next的状态和转换
            std::size_t offset = this->merge_nfa_states(next);

            // 连接当前NFA的最终状态和next的起始状态
            this->add_epsilon_transition(this->get_final(), next.start + offset);

            // 更新当前NFA的最终状态为next的最终状态（加上偏移）
            this->set_final(next.final + offset);
        }

        // 与传入的NFA进行选择操作
        // regexp: 当前NFA|other
        // 将传入的NFA与当前NFA进行选择操作（|）
        void select_with(const nfa& other)
        {
            // 保存当前NFA的原始起始和最终状态
            state::id_type original_start = this->get_start();
            state::id_type original_final = this->get_final();

            // 使用公共方法合并other的状态和转换
            std::size_t offset = this->merge_nfa_states(other);

            // 创建新的起始状态和最终状态
            state::id_type new_start = this->add_state();
            state::id_type new_final = this->add_state();

            // 从新的起始状态到原来的两个NFA的起始状态添加epsilon转换
            this->add_epsilon_transition(new_start, original_start);
            this->add_epsilon_transition(new_start, other.start + offset);

            // 从原来的两个NFA的最终状态到新的最终状态添加epsilon转换
            this->add_epsilon_transition(original_final, new_final);
            this->add_epsilon_transition(other.final + offset, new_final);

            // 更新当前NFA的起始和最终状态
            this->set_start(new_start);
            this->set_final(new_final);
        }

        // 递归解析表达式的主要函数
        static nfa parse_expression(std::string_view& exp)
        {
            if (exp.empty()) {
                nfa empty;
                // 创建一个空NFA（只有起始和结束状态，通过epsilon转换连接）
                empty.add_epsilon_transition(empty.get_start(), empty.get_final());
                return empty;
            }

            nfa result = parse_sequence(exp);

            // 处理选择操作(|)
            while (!exp.empty() && exp[0] == '|') {
                exp.remove_prefix(1); // 跳过'|'
                nfa right = parse_sequence(exp);

                // 创建一个新的NFA来表示选择操作
                nfa choice_nfa;
                std::size_t left_offset  = choice_nfa.merge_nfa_states(result);
                std::size_t right_offset = choice_nfa.merge_nfa_states(right);

                // 创建新的起始和结束状态
                state::id_type new_start = choice_nfa.add_state();
                state::id_type new_final = choice_nfa.add_state();

                // 从新起始状态到左右NFA的起始状态添加epsilon转换
                choice_nfa.add_epsilon_transition(new_start,
                                                  result.get_start() + left_offset);
                choice_nfa.add_epsilon_transition(new_start,
                                                  right.get_start() + right_offset);

                // 从左右NFA的结束状态到新结束状态添加epsilon转换
                choice_nfa.add_epsilon_transition(result.get_final() + left_offset,
                                                  new_final);
                choice_nfa.add_epsilon_transition(right.get_final() + right_offset,
                                                  new_final);

                // 设置新的起始和结束状态
                choice_nfa.set_start(new_start);
                choice_nfa.set_final(new_final);

                result = choice_nfa;
            }

            return result;
        }

        // 解析序列（连接操作）
        static nfa parse_sequence(std::string_view& exp)
        {
            nfa result;

            // 如果表达式为空或以'|'或')'开头，则返回一个空NFA
            if (exp.empty() || exp[0] == '|' || exp[0] == ')') {
                nfa empty;
                empty.add_epsilon_transition(empty.get_start(), empty.get_final());
                return empty;
            }

            // 解析第一个项
            nfa current = parse_term(exp);

            // 继续解析更多项并连接
            while (!exp.empty() && exp[0] != '|' && exp[0] != ')') {
                nfa next = parse_term(exp);

                // 连接当前项和下一项
                std::size_t offset = current.merge_nfa_states(next);

                // 连接当前NFA的最终状态和下一项的起始状态
                current.add_epsilon_transition(current.get_final(),
                                               next.get_start() + offset);

                // 更新当前NFA的最终状态
                current.set_final(next.get_final() + offset);
            }

            return current;
        }

        // 解析单个项（可能包含量词）
        static nfa parse_term(std::string_view& exp)
        {
            nfa base;

            if (exp.empty()) {
                throw nfa::regex_error("Unexpected end of expression");
            }

            char first_char = exp[0];

            // 根据第一个字符决定如何解析
            if (first_char == '(') {
                exp.remove_prefix(1); // 跳过'('
                base = parse_expression(exp);

                if (exp.empty() || exp[0] != ')') {
                    throw nfa::regex_error("Expected closing parenthesis");
                }
                exp.remove_prefix(1); // 跳过')'
            } else if (first_char == '[') {
                base = parse_charset(exp);
            } else if (first_char == '.') {
                base = parse_wildcard(exp);
            } else if (first_char == '\\') {
                if (exp.length() >= 2) {
                    char next_char = exp[1];
                    switch (next_char) {
                        case 'd':
                        case 'D': base = parse_digit(exp); break;
                        case 'w':
                        case 'W': base = parse_word(exp); break;
                        case 's':
                        case 'S': base = parse_space(exp); break;
                        default:
                            base = parse_char(exp); // 处理转义字符
                            break;
                    }
                } else {
                    base = parse_char(exp); // 只有一个反斜杠，当作普通字符
                }
            } else {
                base = parse_char(exp); // 普通字符
            }

            // 检查是否有量词
            if (!exp.empty()) {
                char quantifier = exp[0];
                if (quantifier == '*' || quantifier == '+' || quantifier == '?') {
                    exp.remove_prefix(1); // 跳过量词

                    // 根据量词类型修改NFA
                    state::id_type new_start = base.add_state();
                    state::id_type new_final = base.add_state();

                    if (quantifier == '*') {
                        // a* : 可以匹配0次或多次
                        base.add_epsilon_transition(new_start,
                                                    base.get_start()); // 进入原始NFA
                        base.add_epsilon_transition(base.get_final(),
                                                    new_final); // 从原始结束到新结束
                        base.add_epsilon_transition(base.get_final(),
                                                    base.get_start()); // 循环
                        base.add_epsilon_transition(new_start,
                                                    new_final); // 直接跳过（0次匹配）
                    } else if (quantifier == '+') {
                        // a+ : 至少匹配一次
                        base.add_epsilon_transition(new_start,
                                                    base.get_start()); // 进入原始NFA
                        base.add_epsilon_transition(base.get_final(),
                                                    new_final); // 从原始结束到新结束
                        base.add_epsilon_transition(base.get_final(),
                                                    base.get_start()); // 循环
                    } else if (quantifier == '?') {
                        // a? : 可选，匹配0次或1次
                        base.add_epsilon_transition(new_start,
                                                    base.get_start()); // 进入原始NFA
                        base.add_epsilon_transition(base.get_final(),
                                                    new_final); // 从原始结束到新结束
                        base.add_epsilon_transition(new_start,
                                                    new_final); // 直接跳过（0次匹配）
                    }

                    base.set_start(new_start);
                    base.set_final(new_final);
                }
            }

            return base;
        }

       public:
        static nfa from(std::string_view exp)
        {
            // 使用递归下降解析器解析表达式
            std::string_view exp_copy = exp;
            nfa parsed_nfa            = parse_expression(exp_copy);

            // 检查是否还有未解析的部分
            if (!exp_copy.empty()) {
                throw nfa::regex_error(std::format(
                    "Unexpected characters at end of expression: {}", exp_copy));
            }

            return parsed_nfa;
        }

        class regex_error: public std::runtime_error {
           public:
            explicit regex_error(const std::string& msg): std::runtime_error(msg)
            {
            }
        };
    };

    class dfa {
       private:
       public:
        explicit dfa(const nfa& nfa)
        {
        }

        explicit dfa(const std::string_view exp): dfa(nfa::from(exp))
        {
        }

        static dfa from(const nfa& nfa)
        {
            return dfa(nfa);
        }

        static dfa from(std::string_view exp)
        {
            return dfa(nfa::from(exp));
        }
    };
} // namespace regex
