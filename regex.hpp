#pragma once

#include <algorithm>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iterator>
#include <map>
#include <exception>
#include <regex>
#include <set>
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
        static state::id_type id_counter;
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
    };

    class nfa {
       private:
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

        void add_epsilon_transition(state::id_type state, state::id_type to)
        {
            this->states.at(state).add_epsilon_transition(to);
        }

        void apply_quantifier(char quantifier)
        {
            switch (quantifier) {
                case '?': break;
                case '+': break;
                case '*': {
                    state::id_type new_start = this->add_state();
                    state::id_type new_final = this->add_state();

                    // 连接new_start和start
                    this->add_epsilon_transition(new_start, this->get_start());
                    this->add_epsilon_transition(this->get_final(), new_final);

                    // old_end连接到new_start, 实现多次匹配
                    this->add_epsilon_transition(this->get_final(), new_start);

                    this->set_start(new_start);
                    this->set_final(new_final);
                } break;

                default: break;
            }
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

        // 生成各种基本元素

        // 创建单个字符的NFA
        // regexp: a
        // NFA: start -a-> final
        static nfa create_char(char ch)
        {
            nfa nfa;

            nfa.add_transition(nfa.get_start(), ch, nfa.get_final());

            return nfa;
        }

        // 创建通配符.的NFA
        // regexp: .
        // start -any ascii char-> final
        static nfa create_wildcard(void)
        {
            nfa nfa;

            for (std::size_t i = 0; i < 256; i++) {
                if (regex::ascii_printable_chars.test(i)) {
                    nfa.add_transition(nfa.get_start(), static_cast<char>(i),
                                       nfa.get_final());
                }
            }

            return nfa;
        }

        // 创建通配符\d or \D 的NFA
        // regexp: \d 等价于 [0-9], \D等价于 [^0-9]
        static nfa create_digit(bool is_negated = false)
        {
            return create_charset(is_negated ? "[^0-9]" : "[0-9]");
        }

        // 创建通配符\w or \W 的NFA
        // regexp: \w 等价于 [a-zA-Z0-9_], \W等价于 [^a-zA-Z0-9_]
        static nfa create_word(bool is_negated = false)
        {
            return create_charset(is_negated ? "[^a-zA-Z0-9_]" : "[a-zA-Z0-9_]");
        }

        // 创建通配符\s or \S 的NFA
        // regexp: \s 等价于 [ \t\r\n\b\f\v], \S等价于 [^ \t\r\n\b\f\v]
        static nfa create_space(bool is_negated = false)
        {
            return create_charset(is_negated ? "[ \t\r\n\b\f\v]" : "[^ \t\r\n\b\f\v]");
        }

        // 创建字符集的NFA
        // regexp: [abc]
        // NFA:     |-a-> s0 -|
        // start -> |-b-> s1 -|-> final
        //          |-c-> s2 -|
        static nfa create_charset(std::string_view exp)
        {
            nfa nfa;
            std::bitset<256> chars;

            const auto exp_orig = exp;

            bool is_negated = '^' == exp[1];
            bool is_escape  = false;
            bool has_end    = false;

            exp = exp.substr(is_negated ? 2 : 1);

            for (std::size_t i = 0; i < exp.length(); i++) {
                char current = is_escape ? nfa::handle_escape(exp[i]) : exp[i];

                if (current == ']') {
                    has_end = true;
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

            // todo: debug print {chars}

            std::bitset<256> charset =
                is_negated ? (regex::ascii_printable_chars & ~chars) : chars;

            // 为所有符合条件的字符添加转换规则
            for (std::size_t i = 0; i < 256; i++) {
                if (charset.test(i)) {
                    nfa.add_transition(nfa.get_start(), static_cast<char>(i),
                                       nfa.get_final());
                }
            }

            return nfa;
        }

        // 创建连接关系的NFA
        // regexp: abc
        // NFA: start -a-> s0 -b-> s1 -c-> final
        static nfa create_connect(nfa&& prev, nfa&& next)
        {
            nfa combine;

            return combine;
        }

        // 创建选择关系的NFA
        // regexp: a|bc
        // NFA:      |-a-> s0 ---------|
        // start -$->|                 |-$-> final
        //           |-b-> s1 -c-> s2 -|
        static nfa create_select(nfa&& left, nfa&& right)
        {
            nfa combine;

            return combine;
        }

       public:
        explicit nfa(std::string_view exp)
        {
        }

        static nfa from(std::string_view exp)
        {
            return nfa(exp);
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

        explicit dfa(const std::string_view exp): dfa(nfa(exp))
        {
        }

        static dfa from(const nfa& nfa)
        {
            return dfa(nfa);
        }

        static dfa from(std::string_view exp)
        {
            return from(nfa(exp));
        }
    };

    inline state::id_type state::id_counter = 1;

} // namespace regex
