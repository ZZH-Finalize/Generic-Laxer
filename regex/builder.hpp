#include <string_view>
#include "nfa.hpp"
#include "dfa.hpp"

namespace regex {

    inline nfa build_nfa(std::string_view exp)
    {
        return nfa::builder::build(exp);
    }

    inline dfa build_dfa(const nfa& nfa)
    {
        return dfa::builder::build(nfa);
    }

    static inline dfa build_dfa(std::string_view exp)
    {
        return build_dfa(build_nfa(exp));
    }

    static inline dfa build(const nfa& nfa)
    {
        return build_dfa(nfa);
    }

    static inline dfa build(std::string_view exp)
    {
        return build_dfa(exp);
    }

    inline dfa minimize(const dfa& dfa)
    {
        return dfa::builder::minimize(dfa);
    }

} // namespace regex
