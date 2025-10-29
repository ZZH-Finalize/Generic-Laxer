#include <string_view>
#include "nfa.hpp"
#include "dfa.hpp"

namespace regex {
    class builder {
       public:
        static inline nfa from(std::string_view exp)
        {
            return nfa::builder::build(exp);
        }

        static inline dfa from(const nfa& nfa)
        {
            return dfa::builder::build(nfa);
        }

        static inline dfa minimize(const dfa& dfa)
        {
            return dfa;
        }
    };

    static inline nfa build_nfa(std::string_view exp)
    {
        return builder::from(exp);
    }

    static inline dfa build_dfa(const nfa& nfa)
    {
        return builder::from(nfa);
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

    static inline dfa minimize(const dfa& dfa)
    {
        return builder::minimize(dfa);
    }

} // namespace regex
