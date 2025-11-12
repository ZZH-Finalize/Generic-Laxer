#pragma once

#include <array>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace regex {
    // 判断是否为容器
    template<typename T>
    concept is_container = requires(T t) { typename T::value_type; };

    // 可以执行push_back, 用于支持std::vector这样的容器
    template<typename T>
    concept has_push_back = requires(T t, const T::value_type& v) { t.push_back(v); };

    // 可以执行insert, 用于支持std::set这样的容器
    template<typename T>
    concept has_insert = requires(T t, const T::value_type& v) { t.insert(v); };

    template<typename T>
    class basic_state {
       public:
        // 非容器情况
        template<bool IsContainer, typename U>
        struct id_type_helper
        {
            using type = U;
        };

        // 容器情况
        template<typename U>
        struct id_type_helper<true, U>
        {
            using type = typename U::value_type;
        };

        using id_t = typename id_type_helper<is_container<T>, T>::type;

        using transition_map_item_t = T;
        using transition_map_t =
            std::array<transition_map_item_t,
                       std::numeric_limits<unsigned char>::max() + 1>;

        inline void set_transition(char ch, const T& id)
        {
            this->transition_map[ch] = id;
        }

        inline void set_transition(char ch, T&& id)
        {
            this->transition_map[ch] = std::move(id);
        }

        inline void add_transition(char input, id_t to)
        requires has_push_back<T>
        {
            this->transition_map[input].push_back(to);
        }

        inline void add_transition(char input, id_t to)
        requires has_insert<T>
        {
            this->transition_map[input].insert(to);
        }

        inline const auto& get_transition(char ch) const
        {
            return this->transition_map[ch];
        }

        inline const auto& get_transition_map(void) const noexcept
        {
            return this->transition_map;
        }

       protected:
        transition_map_t transition_map;
    };

} // namespace regex
