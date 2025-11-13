#include "nfa.hpp"
#include "builder.hpp"

namespace regex {

    char builder::handle_escape(char ch)
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

    nfa builder::parse_char(std::string_view& exp)
    {
        if (exp.empty()) {
            throw nfa::regex_error("Unexpected end of expression");
        }

        char ch = exp[0];
        if (ch == '\\') {
            if (exp.length() < 2) {
                throw nfa::regex_error("Unexpected end of expression after escape");
            }

            ch = builder::handle_escape(exp[1]);
            exp.remove_prefix(2); // 跳过转义字符
        } else {
            exp.remove_prefix(1); // 跳过普通字符
        }

        nfa result;
        result.add_transition(result.get_start(), ch, result.get_final());
        return result;
    }

    nfa builder::parse_wildcard(std::string_view& exp)
    {
        if (exp.empty() or exp[0] != '.') {
            throw nfa::regex_error("Expected wildcard character '.'");
        }
        exp.remove_prefix(1); // 跳过'.'字符

        nfa result;
        result.add_transition(regex::ascii_printable_chars);

        return result;
    }

    nfa builder::parse_charset(std::string_view& exp)
    {
        if (exp.empty() or exp[0] != '[') {
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
            char current = is_escape ? builder::handle_escape(exp[i]) : exp[i];

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
                        range_end = builder::handle_escape(exp[i + 3]);
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

    nfa builder::parse_digit(std::string_view& exp)
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

    nfa builder::parse_word(std::string_view& exp)
    {
        if (exp.length() < 2 or exp[0] != '\\' or (exp[1] != 'w' and exp[1] != 'W')) {
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

    nfa builder::parse_space(std::string_view& exp)
    {
        if (exp.length() < 2 or exp[0] != '\\' or (exp[1] != 's' and exp[1] != 'S')) {
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

    nfa builder::parse_expression(std::string_view& exp)
    {
        if (exp.empty()) {
            nfa empty;
            // 创建一个空NFA（只有起始和结束状态，通过epsilon转换连接）
            empty.add_epsilon_transition(empty.get_start(), empty.get_final());
            return empty;
        }

        nfa result = parse_sequence(exp);

        // 处理选择操作(|)
        while (not exp.empty() and exp[0] == '|') {
            exp.remove_prefix(1); // 跳过'|'
            nfa right = parse_sequence(exp);

            // 创建一个新的NFA来表示选择操作
            nfa choice_nfa;
            std::size_t left_offset  = choice_nfa.merge_states(result);
            std::size_t right_offset = choice_nfa.merge_states(right);

            // 创建新的起始和结束状态
            id_t new_start = choice_nfa.add_state();
            id_t new_final = choice_nfa.add_state();

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

    nfa builder::parse_sequence(std::string_view& exp)
    {
        nfa result;

        // 如果表达式为空或以'|'或')'开头，则返回一个空NFA
        if (exp.empty() or exp[0] == '|' or exp[0] == ')') {
            nfa empty;
            empty.add_epsilon_transition(empty.get_start(), empty.get_final());
            return empty;
        }

        // 解析第一个项
        nfa current = parse_term(exp);

        // 继续解析更多项并连接
        while (!exp.empty() and exp[0] != '|' and exp[0] != ')') {
            nfa next = parse_term(exp);

            // 连接当前项和下一项
            std::size_t offset = current.merge_states(next);

            // 连接当前NFA的最终状态和下一项的起始状态
            current.add_epsilon_transition(current.get_final(),
                                           next.get_start() + offset);

            // 更新当前NFA的最终状态
            current.set_final(next.get_final() + offset);
        }

        return current;
    }

    nfa builder::parse_term(std::string_view& exp)
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

            if (exp.empty() or exp[0] != ')') {
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
            if (quantifier == '*' or quantifier == '+' or quantifier == '?') {
                exp.remove_prefix(1); // 跳过量词

                // 根据量词类型修改NFA
                // 保存原始的start和final状态ID，因为add_state会改变base的状态
                id_t original_start = base.get_start();
                id_t original_final = base.get_final();

                id_t new_start = base.add_state();
                id_t new_final = base.add_state();

                if (quantifier == '*') {
                    // a* : 可以匹配0次或多次
                    base.add_epsilon_transition(new_start,
                                                original_start); // 进入原始NFA
                    base.add_epsilon_transition(original_final,
                                                new_final); // 从原始结束到新结束
                    base.add_epsilon_transition(original_final,
                                                original_start); // 循环
                    base.add_epsilon_transition(new_start,
                                                new_final); // 直接跳过（0次匹配）
                } else if (quantifier == '+') {
                    // a+ : 至少匹配一次
                    base.add_epsilon_transition(new_start,
                                                original_start); // 进入原始NFA
                    base.add_epsilon_transition(original_final,
                                                new_final); // 从原始结束到新结束
                    base.add_epsilon_transition(original_final,
                                                original_start); // 循环
                } else if (quantifier == '?') {
                    // a? : 可选，匹配0次或1次
                    base.add_epsilon_transition(new_start,
                                                original_start); // 进入原始NFA
                    base.add_epsilon_transition(original_final,
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

    nfa builder::build(std::string_view exp)
    {
        // 使用递归下降解析器解析表达式
        std::string_view exp_copy = exp;
        nfa parsed_nfa            = parse_expression(exp_copy);

        // 检查是否还有未解析的部分
        if (not exp_copy.empty()) {
            throw nfa::regex_error(
                std::format("Unexpected characters at end of expression: {}", exp_copy));
        }

        return parsed_nfa;
    }

} // namespace regex
