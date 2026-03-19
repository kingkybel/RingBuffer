/*
 * Repository:  https://github.com/kingkybel/TypeTraits
 * File Name:   include/traits.h
 * Description: Macros to define traits that can static_assert whether
 *              static or non static member-functions are present in a type.
 *
 * Copyright (C) 2024 Dieter J Kybelksties <github@kybelksties.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * @date: 2024-11-03
 * @author: Dieter J Kybelksties
 */

#ifndef TRAITS_H_INCLUDED
#define TRAITS_H_INCLUDED

#include <cstdint>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#if __cplusplus >= 202'002L
    #include <concepts>
#endif

namespace util
{

template <typename> struct is_tuple : std::false_type
{
};

template <typename... T> struct is_tuple<std::tuple<T...>> : std::true_type
{
};

template <typename... T> using is_tuple_t = typename is_tuple<T...>::type;

template <typename... T> bool constexpr is_tuple_v = is_tuple<T...>::value;

#if __cplusplus >= 202'002L
// C++20 Implementation using Concepts
template <typename T, typename EqualTo = T>
concept equality_comparable = requires(T a, EqualTo b) {
    { a == b } -> std::convertible_to<bool>;
};

template <typename T, typename LessThan = T>
concept less_comparable = requires(T a, LessThan b) {
    { a < b } -> std::convertible_to<bool>;
};

template <typename T>
concept is_tuple_c = is_tuple_v<T>;

#endif

// Helper for equality comparability (C++14/17 fallback)
template <typename T, typename EqualTo, typename = void> struct is_equality_comparable : std::false_type
{
};

// Specialization when decltype(std::declval<U>() == std::declval<V>()) is valid and returns bool
template <typename T, typename EqualTo>
struct is_equality_comparable<T, EqualTo, std::void_t<decltype(std::declval<T>() == std::declval<EqualTo>())>>
    : std::is_same<bool, decltype(std::declval<T>() == std::declval<EqualTo>())>
{
};

// Public trait for equality comparability
template <typename T, typename EqualTo = T>
using is_equality_comparable_t = typename is_equality_comparable<T, EqualTo>::type;

// Public trait for equality comparability
template <typename T, typename EqualTo = T>
bool constexpr is_equality_comparable_v = is_equality_comparable<T, EqualTo>::value;

// Helper for less-than comparability
template <typename T, typename LessThan, typename = void> struct is_less_comparable : std::false_type
{
};

// Specialization when decltype(std::declval<U>() < std::declval<V>()) is valid and returns bool
template <typename T, typename LessThan>
struct is_less_comparable<T, LessThan, std::void_t<decltype(std::declval<T>() < std::declval<LessThan>())>>
    : std::is_same<bool, decltype(std::declval<T>() < std::declval<LessThan>())>
{
};

// Public trait for less-than comparability
template <typename T, typename LessThan = T>
using is_less_comparable_t = typename is_less_comparable<T, LessThan>::type;

template <typename T, typename LessThan = T>
bool constexpr is_less_comparable_v = is_less_comparable<T, LessThan>::value;

template <size_t N, typename T, typename... types> struct get_Nth_type
{
    using type = typename get_Nth_type<N - 1, types...>::type;
};

template <typename T, typename... types> struct get_Nth_type<0, T, types...>
{
    using type = T;
};

/**
 * @brief Trait to identify custom character types. Default is false.
 */
template <typename> struct is_char : std::false_type
{
};

/**
 * @brief Trait to identify custom character types. Specialization for "char" true.
 */
template <> struct is_char<char> : std::true_type
{
};

/**
 * @brief Trait to identify custom character types. Specialization for "wchar_t" true.
 */
template <> struct is_char<wchar_t> : std::true_type
{
};

/**
 * @brief Trait to identify custom character types. Specialization for "char16_t" true.
 */
template <> struct is_char<char16_t> : std::true_type
{
};

/**
 * @brief Trait to identify custom character types. Specialization for "char32_t" true.
 */
template <> struct is_char<char32_t> : std::true_type
{
};

template <typename T> using is_char_t = typename is_char<T>::type;

template <typename T> bool constexpr is_char_v = is_char<T>::value;

/**
 * @brief Trait to identify std-string types. Default is false.
 */
template <typename> struct is_std_string : std::false_type
{
    using char_type = void;
};

/**
 * @brief Trait to identify std-string types. Specialization for "std::basic_string" is true.
 */
template <typename CharT, typename Traits, typename Alloc>
struct is_std_string<std::basic_string<CharT, Traits, Alloc>> : std::true_type
{
    using char_type = CharT;
};

template <typename T> using is_std_string_t = typename is_std_string<T>::type;

template <typename T> bool constexpr is_std_string_v = is_std_string<T>::value;

/**
 * @brief Convert a char to a different char type
 *
 * @tparam CharTto_ type to convert to
 * @tparam CharTfrom_ type to convert from
 * @param c the char to convert
 * @return constexpr CharTto_ the converted char
 */
template <typename CharTto_, typename CharTfrom_>
requires util::is_char_v<CharTto_> && util::is_char_v<CharTfrom_>
constexpr CharTto_ charToChar(CharTfrom_ c)
{
    return static_cast<CharTto_>(c);
}

/**
 * @brief Trait to identify std-string-view types. Default is false.
 */
template <typename U> struct is_std_string_view : std::false_type
{
    using char_type = void;
};

/**
 * @brief Trait to identify std-string types. Specialization for "std::basic_string_view" is true.
 */
template <typename CharT, typename Traits>
struct is_std_string_view<std::basic_string_view<CharT, Traits>> : std::true_type
{
    using char_type = CharT;
};

template <typename T> using is_std_string_view_t = typename is_std_string_view<T>::type;

template <typename T> bool constexpr is_std_string_view_v = is_std_string_view<T>::value;

/**
 * @brief Trait to identify character pointer types. Default is false.
 */
template <typename U> struct is_char_pointer : std::false_type
{
    using char_type = void;
};

/**
 * @brief Trait to identify character pointer types. Specialization for "CharT*" is true, if CharT is a character.
 */
template <typename CharT> struct is_char_pointer<CharT*>
{
    static constexpr bool value = is_char_v<CharT>;
    using char_type             = typename std::conditional_t<value, CharT, void>;
};

/**
 * @brief Trait to identify character pointer types. Specialization for "const CharT*" is true, if CharT is a character.
 */
template <typename CharT> struct is_char_pointer<CharT const*>
{
    static constexpr bool value = is_char_v<CharT>;
    using char_type             = typename std::conditional_t<value, CharT, void>;
};

/**
 * @brief Trait to identify character pointer types. Specialization for "const CharT*&" is true, if CharT is a
 * character.
 */
template <typename CharT> struct is_char_pointer<CharT const*&>
{
    static constexpr bool value = is_char_v<CharT>;
    using char_type             = typename std::conditional_t<value, CharT, void>;
};

/**
 * @brief Trait to identify character pointer types. Specialization for "CharT*&" is true, if CharT is a
 * character.
 */
template <typename CharT> struct is_char_pointer<CharT*&>
{
    static constexpr bool value = is_char_v<CharT>;
    using char_type             = typename std::conditional_t<value, CharT, void>;
};

/**
 * @brief Trait to identify character pointer types. Specialization for "const CharT* const" is true, if CharT is a
 * character.
 */
template <typename CharT> struct is_char_pointer<CharT const* const>
{
    static constexpr bool value = is_char_v<CharT>;
    using char_type             = typename std::conditional_t<value, CharT, void>;
};

template <typename CharT> using is_char_pointer_t = typename is_char_pointer<CharT>::type;

template <typename CharT> bool constexpr is_char_pointer_v = is_char_pointer<CharT>::value;

/**
 * @brief Trait to identify character array types. Default is false.
 */
template <typename U> struct is_char_array : std::false_type
{
    using char_type = void;
};

/**
 * @brief Trait to identify character array types. Specialization for "CharT[]" is true, if CharT is a character.
 */
template <typename CharT> struct is_char_array<CharT[]>
{
    static constexpr bool value = is_char_v<CharT>;
    using char_type             = typename std::conditional_t<value, CharT, void>;
};

/**
 * @brief Trait to identify character array types. Specialization for "CharT[sz]" is true, if CharT is a character.
 */
template <typename CharT, size_t sz> struct is_char_array<CharT[sz]>
{
    static constexpr bool value = is_char_v<CharT>;
    using char_type             = typename std::conditional_t<value, CharT, void>;
};

/**
 * @brief Trait to identify character array types. Specialization for "const CharT[]" is true, if CharT is a character.
 */
template <typename CharT> struct is_char_array<CharT const[]>
{
    static constexpr bool value = is_char_v<CharT>;
    using char_type             = typename std::conditional_t<value, CharT, void>;
};

/**
 * @brief Trait to identify character array types. Specialization for "const CharT[sz]" is true, if CharT is a
 * character.
 */
template <typename CharT, size_t sz> struct is_char_array<CharT const[sz]>
{
    static constexpr bool value = is_char_v<CharT>;
    using char_type             = typename std::conditional_t<value, CharT, void>;
};

template <typename CharT> using is_char_array_t = typename is_char_array<CharT>::type;

template <typename CharT> bool constexpr is_char_array_v = is_char_array<CharT>::value;

/**
 * @brief Trait to identify "logical strings": char* char-arrays, string_views and strings.
 *
 * @tparam T type to check
 */
template <typename T> struct is_string
{
    // Check for custom string-like types
    static constexpr bool is_std_string_val      = is_std_string_v<T>;
    static constexpr bool is_std_string_view_val = is_std_string_view_v<T>;
    static constexpr bool is_char_pointer_val    = is_char_pointer_v<T>;
    static constexpr bool is_char_array_val      = is_char_array_v<T>;

    static constexpr bool value =
        is_std_string_val || is_std_string_view_val || is_char_pointer_val || is_char_array_val;
    using __str_char_type    = typename is_std_string<T>::char_type;
    using __str_vw_char_type = typename is_std_string_view<T>::char_type;
    using __pchr_char_type   = typename is_char_pointer<T>::char_type;
    using __Achr_char_type   = typename is_char_array<T>::char_type;

    // clang-format off
    using char_type = typename std::conditional_t<is_std_string_val,
                                       __str_char_type,
                                       typename std::conditional_t<is_std_string_view_val,
                                                          __str_vw_char_type,
                                                          typename std::conditional_t<is_char_pointer_val,
                                                                             __pchr_char_type,
                                                                             __Achr_char_type>>>;
    // clang-format on
};

template <typename T> using is_string_t = typename is_string<T>::type;

template <typename T> bool constexpr is_string_v = is_string<T>::value;

template <typename StringT1_, typename StringT2_> struct is_compatible_string
{
    static constexpr bool is_string_1    = is_string_v<StringT1_>;
    static constexpr bool is_string_2    = is_string_v<StringT2_>;
    using char_type_1                    = typename is_string<StringT1_>::char_type;
    using char_type_2                    = typename is_string<StringT2_>::char_type;
    static constexpr bool same_char_type = std::is_same_v<char_type_1, char_type_2>;

    constexpr static bool value = is_string_1 && is_string_2 && same_char_type;
    using type                  = std::conditional_t<value, char_type_1, void>;
};

template <typename StringT1_, typename StringT2_>
using is_compatible_string_t = typename is_compatible_string<StringT1_, StringT2_>::type;

template <typename StringT1_, typename StringT2_>
bool constexpr is_compatible_string_v = is_compatible_string<StringT1_, StringT2_>::value;

template <typename StringT_, typename StringOrCharT_> struct has_std_string_compatible_char
{
    // clang-format off
    using string_char_type = typename is_string<StringT_>::char_type ;

    static constexpr bool is_std_string_1st_val    = is_std_string_v<StringT_>;
    static constexpr bool is_compatible_string_val = is_compatible_string_v<StringT_, StringOrCharT_>;
    static constexpr bool is_char_2nd_val          = is_char_v<StringOrCharT_>;
    static constexpr bool is_compatible_char_val   = is_char_2nd_val && std::is_same_v<string_char_type, StringOrCharT_>;

    constexpr static bool value                  = is_compatible_string_val || is_compatible_char_val;
    using type = std::conditional_t<value, string_char_type, void>;
    // clang-format on
};

template <typename StringT1_, typename StringOrCharT_>
using has_std_string_compatible_char_t = typename has_std_string_compatible_char<StringT1_, StringOrCharT_>::type;

template <typename StringT1_, typename StringOrCharT_>
bool constexpr has_std_string_compatible_char_v = has_std_string_compatible_char<StringT1_, StringOrCharT_>::value;

template <typename T>
requires util::is_std_string_v<T> || util::is_std_string_view_v<T>
size_t string_or_char_size(T const& str)
{
    return std::ssize(str);
}

template <typename T>
requires util::is_char_pointer_v<T> || util::is_char_array_v<T>
size_t string_or_char_size(T const& str)
{
    using char_type = util::is_string<T>::char_type;
    return std::char_traits<char_type>::length(str);
}

template <typename T>
requires util::is_char_v<T>
size_t string_or_char_size(T const&)
{
    return 1UL;
}

template <typename T>
requires(!util::is_string_v<T> && !util::is_char_v<T>)
size_t string_or_char_size(T const& /* str */)
{
    return 0UL;
}

// Primary template to check if std::hash<Type> exists and is callable with const Type&
template <typename Type, typename = void> struct has_std_hash : std::false_type
{
};

// Specialization for when std::hash<Type> is well-formed and callable
template <typename Type>
struct has_std_hash<Type, std::void_t<decltype(std::declval<std::hash<Type>>()(std::declval<Type const&>()))>>
    : std::is_same<decltype(std::declval<std::hash<Type>>()(std::declval<Type const&>())), size_t>
{
};

template <typename Type> using has_std_hash_t = typename has_std_hash<Type>::type;

// Helper variable template (for C++14 and above)
template <typename Type> constexpr bool has_std_hash_v = has_std_hash<Type>::value;

} //  namespace util

#endif // TRAITS_H_INCLUDED
