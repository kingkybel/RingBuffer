/*
 * Repository:  https://github.com/kingkybel/TypeTraits
 * File Name:   test/traits_tests.cc
 * Description: tests for type-traits.
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
#include "traits.h"
#include "traits_static.h"

#include <gtest/gtest.h>
#include <string>
#include <tuple>

using namespace std;
using namespace util;

class TraitsTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // just in case
    }

    void TearDown() override
    {
        // just in case
    }
};

TEST_F(TraitsTest, is_equality_comparable_test)
{
    ASSERT_TRUE(is_equality_comparable_v<std::string>);

    struct NoEqual
    {
    };

    ASSERT_FALSE(is_equality_comparable_v<NoEqual>);

    struct Equal
    {
        bool operator==(Equal const& rhs) const // NOSONAR
        {
            return true;
        }
    };

    ASSERT_TRUE(is_equality_comparable_v<Equal>);
}

TEST_F(TraitsTest, is_less_comparable_test)
{
    ASSERT_TRUE(is_less_comparable_v<double>);

    struct NoLess
    {
    };

    ASSERT_FALSE(is_less_comparable_v<NoLess>);

    struct Less
    {
        bool operator<(Less const& rhs) // NOSONAR
        {
            return true;
        }
    };

    ASSERT_TRUE(is_less_comparable_v<Less>);
}

TEST_F(TraitsTest, is_tuple_test)
{
    using TupleType = std::tuple<double, int, std::string>;
    ASSERT_TRUE(is_tuple_v<TupleType>);
    using PairType = std::pair<std::string, int>;
    ASSERT_FALSE(is_tuple_v<PairType>);
    ASSERT_FALSE(is_tuple_v<double>);

    struct NoTuple
    {
        int    i;
        double f;
    };

    ASSERT_FALSE(is_tuple_v<NoTuple>);
}

TEST_F(TraitsTest, is_char_test)
{
    ASSERT_TRUE(is_char_v<char>);
    ASSERT_TRUE(is_char_v<char16_t>);
    ASSERT_TRUE(is_char_v<char32_t>);
    ASSERT_FALSE(is_char_v<char[]>);
    ASSERT_FALSE(is_char_v<char16_t[]>);
    ASSERT_FALSE(is_char_v<char32_t[]>);
    ASSERT_FALSE(is_char_v<int>);
}

TEST_F(TraitsTest, is_std_string_test)
{
    ASSERT_FALSE(is_std_string_v<double>);
    ASSERT_FALSE(is_std_string_v<char>);
    ASSERT_FALSE(is_std_string_v<char16_t>);
    ASSERT_FALSE(is_std_string_v<char32_t>);
    ASSERT_FALSE(is_std_string_v<char[]>);
    ASSERT_FALSE(is_std_string_v<char16_t[]>);
    ASSERT_FALSE(is_std_string_v<char32_t[]>);
    ASSERT_FALSE(is_std_string_v<char*>);
    ASSERT_FALSE(is_std_string_v<char16_t*>);
    ASSERT_FALSE(is_std_string_v<char32_t*>);
    ASSERT_TRUE(is_std_string_v<std::string>);
    ASSERT_TRUE(is_std_string_v<std::basic_string<char16_t>>);
    ASSERT_TRUE(is_std_string_v<std::basic_string<char32_t>>);
}

TEST_F(TraitsTest, is_std_string_view_test)
{
    ASSERT_FALSE(is_std_string_view_v<double>);
    ASSERT_FALSE(is_std_string_view_v<char>);
    ASSERT_FALSE(is_std_string_view_v<char16_t>);
    ASSERT_FALSE(is_std_string_view_v<char32_t>);
    ASSERT_FALSE(is_std_string_view_v<char[]>);
    ASSERT_FALSE(is_std_string_view_v<char16_t[]>);
    ASSERT_FALSE(is_std_string_view_v<char32_t[]>);
    ASSERT_FALSE(is_std_string_view_v<char*>);
    ASSERT_FALSE(is_std_string_view_v<char16_t*>);
    ASSERT_FALSE(is_std_string_view_v<char32_t*>);
    ASSERT_TRUE(is_std_string_view_v<std::string_view>);
    ASSERT_TRUE(is_std_string_view_v<std::basic_string_view<char16_t>>);
    ASSERT_TRUE(is_std_string_view_v<std::basic_string_view<char32_t>>);
}

TEST_F(TraitsTest, is_char_pointer_test)
{
    ASSERT_FALSE(is_char_pointer_v<double>);
    ASSERT_FALSE(is_char_pointer_v<char>);
    ASSERT_FALSE(is_char_pointer_v<char16_t>);
    ASSERT_FALSE(is_char_pointer_v<char32_t>);
    ASSERT_FALSE(is_char_pointer_v<char[]>);
    ASSERT_FALSE(is_char_pointer_v<char16_t[]>);
    ASSERT_FALSE(is_char_pointer_v<char32_t[]>);
    ASSERT_TRUE(is_char_pointer_v<char*>);
    ASSERT_TRUE(is_char_pointer_v<char16_t*>);
    ASSERT_TRUE(is_char_pointer_v<char32_t*>);
    ASSERT_TRUE(is_char_pointer_v<char const*>);
    ASSERT_TRUE(is_char_pointer_v<char16_t const*>);
    ASSERT_TRUE(is_char_pointer_v<char32_t const*>);
    ASSERT_TRUE(is_char_pointer_v<char*&>);
    ASSERT_TRUE(is_char_pointer_v<char16_t*&>);
    ASSERT_TRUE(is_char_pointer_v<char32_t*&>);
    ASSERT_FALSE(is_char_pointer_v<std::string_view>);
    ASSERT_FALSE(is_char_pointer_v<std::basic_string_view<char16_t>>);
    ASSERT_FALSE(is_char_pointer_v<std::basic_string_view<char32_t>>);
}

TEST_F(TraitsTest, is_char_array_test)
{
    ASSERT_FALSE(is_char_array_v<double>);
    ASSERT_FALSE(is_char_array_v<char>);
    ASSERT_FALSE(is_char_array_v<char16_t>);
    ASSERT_FALSE(is_char_array_v<char32_t>);
    ASSERT_TRUE(is_char_array_v<char[]>);
    ASSERT_TRUE(is_char_array_v<char16_t[]>);
    ASSERT_TRUE(is_char_array_v<char32_t[]>);
    ASSERT_TRUE(is_char_array_v<char const[]>);
    ASSERT_TRUE(is_char_array_v<char16_t const[]>);
    ASSERT_TRUE(is_char_array_v<char32_t const[]>);
    ASSERT_FALSE(is_char_array_v<char*>);
    ASSERT_FALSE(is_char_array_v<char16_t*>);
    ASSERT_FALSE(is_char_array_v<char32_t*>);
    ASSERT_FALSE(is_char_array_v<char const*>);
    ASSERT_FALSE(is_char_array_v<char16_t const*>);
    ASSERT_FALSE(is_char_array_v<char32_t const*>);
    ASSERT_FALSE(is_char_array_v<char*&>);
    ASSERT_FALSE(is_char_array_v<char16_t*&>);
    ASSERT_FALSE(is_char_array_v<char32_t*&>);
    ASSERT_FALSE(is_char_array_v<std::string_view>);
    ASSERT_FALSE(is_char_array_v<std::basic_string_view<char16_t>>);
    ASSERT_FALSE(is_char_array_v<std::basic_string_view<char32_t>>);
}

TEST_F(TraitsTest, is_string_test)
{
    ASSERT_FALSE(is_string_v<double>);
    ASSERT_FALSE(is_string_v<char>);
    ASSERT_FALSE(is_string_v<char16_t>);
    ASSERT_FALSE(is_string_v<char32_t>);
    ASSERT_TRUE(is_string_v<char[]>);
    ASSERT_TRUE(is_string_v<char16_t[]>);
    ASSERT_TRUE(is_string_v<char32_t[]>);
    ASSERT_TRUE(is_string_v<char const[]>);
    ASSERT_TRUE(is_string_v<char16_t const[]>);
    ASSERT_TRUE(is_string_v<char32_t const[]>);
    ASSERT_TRUE(is_string_v<char*>);
    ASSERT_TRUE(is_string_v<char16_t*>);
    ASSERT_TRUE(is_string_v<char32_t*>);
    ASSERT_TRUE(is_string_v<char const*>);
    ASSERT_TRUE(is_string_v<char16_t const*>);
    ASSERT_TRUE(is_string_v<char32_t const*>);
    ASSERT_TRUE(is_string_v<char*&>);
    ASSERT_TRUE(is_string_v<char16_t*&>);
    ASSERT_TRUE(is_string_v<char32_t*&>);
    ASSERT_TRUE(is_string_v<std::string_view>);
    ASSERT_TRUE(is_string_v<std::basic_string_view<char16_t>>);
    ASSERT_TRUE(is_string_v<std::basic_string_view<char32_t>>);
}

TEST_F(TraitsTest, is_compatible_string_test)
{
    // clang-format off
    // formatter remove double round brackets
    ASSERT_TRUE((is_compatible_string_v<std::string, char*>));
    ASSERT_TRUE((std::is_same_v<is_compatible_string<std::string, char32_t*>::char_type_1, char>));
    ASSERT_TRUE((std::is_same_v<is_compatible_string<std::string, char32_t*>::char_type_2, char32_t>));
    ASSERT_FALSE((std::is_same_v<is_compatible_string<std::string, char32_t*>::char_type_1, char32_t>));
    ASSERT_FALSE((std::is_same_v<is_compatible_string<std::string, char32_t*>::char_type_2, char>));
    ASSERT_FALSE((is_compatible_string<std::string, char32_t*>::same_char_type));
    ASSERT_FALSE((is_compatible_string_v<std::string, char32_t*>));
    // clang-format on
}

TEST_F(TraitsTest, has_std_string_compatible_char_test)
{
    // clang-format off
    // formatter remove double round brackets
    ASSERT_TRUE((has_std_string_compatible_char_v<std::string, char>));
    ASSERT_TRUE((has_std_string_compatible_char_v<std::string, char*>));
    ASSERT_TRUE((has_std_string_compatible_char_v<std::string, const char[]>));
    ASSERT_FALSE((has_std_string_compatible_char_v<char,std::string>)) << "first template is not a string so should return false";
    ASSERT_TRUE((has_std_string_compatible_char_v<char*, std::string>));
    ASSERT_TRUE((has_std_string_compatible_char_v<const char[], std::string>));
    // clang-format on
}

TEST_F(TraitsTest, string_or_char_size_test)
{
    ASSERT_EQ(string_or_char_size(0.0), 0UL);
    ASSERT_EQ(string_or_char_size('a'), 1UL);
    ASSERT_EQ(string_or_char_size("abc"), 3UL);
    ASSERT_EQ(string_or_char_size(std::string("abcefg")), 6UL);
}

TEST_F(TraitsTest, has_std_hash_test)
{
    ASSERT_TRUE(has_std_hash_v<double>);
    ASSERT_TRUE(has_std_hash_v<int>);
    ASSERT_TRUE(has_std_hash_v<std::string>);
    ASSERT_TRUE(has_std_hash_v<void*>);
    ASSERT_FALSE(has_std_hash_v<void>);

    struct NoHash
    {
    };

    ASSERT_FALSE(has_std_hash_v<NoHash>);
}

TEST_F(TraitsTest, get_Nth_type_test)
{
    using First = get_Nth_type<0, int, double, std::string>::type;
    using Last  = get_Nth_type<2, int, double, std::string>::type;
    ASSERT_TRUE((std::is_same_v<First, int>));
    ASSERT_TRUE((std::is_same_v<Last, std::string>));
}

TEST_F(TraitsTest, charToChar_test)
{
    ASSERT_EQ(charToChar<char16_t>('A'), u'A');
    ASSERT_EQ(charToChar<char>('z'), 'z');
}

TEST_F(TraitsTest, is_char_pointer_const_pointer_test)
{
    using const_char_ptr_const = char const* const;
    ASSERT_TRUE((is_char_pointer_v<const_char_ptr_const>));
}

TEST_F(TraitsTest, is_char_array_bounded_test)
{
    ASSERT_TRUE((is_char_array_v<char[4]>));
    ASSERT_TRUE((is_char_array_v<char const[7]>));
}

namespace
{
struct StaticMemberMatch
{
    static bool fill()
    {
        return true;
    }
};

struct StaticMemberMismatch
{
    static double fill()
    {
        return 1.0;
    }
};

struct MemberMatch
{
    bool fill() // NOSONAR
    {
        return true;
    }
};

struct MemberMismatch
{
    double fill() const
    {
        return 1.0;
    }
};

DEFINE_HAS_STATIC_MEMBER_FUNCTION(has_static_bool_fill, T::fill, bool (*)(void))
DEFINE_HAS_MEMBER_FUNCTION(has_bool_fill, fill, bool (T::*)(void))
} // namespace

TEST_F(TraitsTest, has_static_member_function_macro_test)
{
    ASSERT_TRUE((has_static_bool_fill_v<StaticMemberMatch>));
    ASSERT_FALSE((has_static_bool_fill_v<StaticMemberMismatch>));
}

TEST_F(TraitsTest, has_member_function_macro_test)
{
    ASSERT_TRUE((has_bool_fill_v<MemberMatch>));
    ASSERT_FALSE((has_bool_fill_v<MemberMismatch>));
}
