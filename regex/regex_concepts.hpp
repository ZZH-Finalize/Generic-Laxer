#pragma once

#include <type_traits>

#include "regex_typedef.hpp"

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

    // 可以执行emplace_back 用于支持std::set这样的容器
    template<typename T>
    concept has_emplace_back =
        requires(T t, const T::value_type& v) { t.emplace_back(v); };

    // 非容器情况
    template<bool IsContainer, typename U>
    struct type_in_container
    {
        using value = U;
    };

    // 容器情况
    template<typename U>
    struct type_in_container<true, U>
    {
        using value = typename U::value_type;
    };

    // 判断T能否作为自动机终态类型
    template<typename T>
    concept is_fa_final_state_t =
        std::is_convertible_v<T, id_t> and std::is_constructible_v<T, id_t>
        and std::is_assignable_v<T&, id_t>;

    // 当T是容器时, 判断T内部类型能否作为自动机终态类型
    template<typename T>
    concept is_fa_final_state_container =
        is_container<T> and is_fa_final_state_t<typename T::value_type>;

    template<typename T>
    concept is_fa_final_state = is_fa_final_state_t<T> or is_fa_final_state_container<T>;

} // namespace regex
