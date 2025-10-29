#include <string_view>
#include <type_traits>
#include "nfa.hpp"
#include "dfa.hpp"

namespace regex {

    inline nfa build_nfa(std::string_view exp)
    {
        return nfa::builder::build(exp);
    }

    inline dfa to_dfa(const nfa& nfa)
    {
        return dfa::builder::build(nfa);
    }

    static inline dfa build_dfa(std::string_view exp)
    {
        return to_dfa(build_nfa(exp));
    }

    static inline dfa build(const nfa& nfa)
    {
        return to_dfa(nfa);
    }

    static inline dfa build(std::string_view exp)
    {
        return build_dfa(exp);
    }

    inline dfa minimize(const dfa& dfa)
    {
        return dfa::builder::minimize(dfa);
    }

    template<typename T>
    concept regexp_res = std::is_same_v<T, nfa> || std::is_same_v<T, dfa>;

    // 2. 定义 regexp_op：F 必须能接受 T 并返回 dfa
    template<typename F, typename T>
    concept regexp_op = regexp_res<T> && std::invocable<F, const T&>
                        && std::same_as<std::invoke_result_t<F, const T&>, dfa>;

    template<typename T, typename F>
    requires regexp_res<T> && regexp_op<F, T>
    inline dfa operator|(const T& input, F f)
    {
        return f(input);
    }

} // namespace regex
