//#############################################################################
// Eggs.Storage
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2019
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HPX_UTIL_STORAGE_HPP
#define HPX_UTIL_STORAGE_HPP

#include <hpx/config.hpp>

#include <cstddef>
#include <initializer_list>
#include <new>
#include <type_traits>
#include <utility>

#if __cplusplus < 201103L
#  if !defined(_MSC_VER) || _MSC_VER < 1900
#    error Eggs.Storage requires compiler and library support for the ISO C++ 2011 standard.
#  endif
#endif

/// constexpr support
#if __cplusplus >= 201402L
#  define EGGS_CXX14_CONSTEXPR constexpr
#elif __cpp_constexpr >= 201304L
#  define EGGS_CXX14_CONSTEXPR constexpr
#else
#  define EGGS_CXX14_CONSTEXPR
#endif

namespace hpx { namespace util
{
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
#if __cpp_lib_byte >= 201603L
        using std::byte;
#else
        using byte = unsigned char;
#endif

        ///////////////////////////////////////////////////////////////////////
#if __cpp_lib_launder >= 201606L
        using std::launder;
#else
        template <typename T>
        constexpr T* launder(T* p) noexcept
        {
            return p;
        }
#endif

        ///////////////////////////////////////////////////////////////////////
        template <bool C>
        struct conditionally_copyable
        {};

        template <>
        struct conditionally_copyable<false>
        {
            conditionally_copyable() noexcept = default;
            conditionally_copyable(conditionally_copyable const&) = delete;
            conditionally_copyable& operator=(conditionally_copyable const&) = delete;
        };

        ///////////////////////////////////////////////////////////////////////
        template <std::size_t... Vs>
        struct static_max;

        template <>
        struct static_max<>
          : std::integral_constant<std::size_t, 0>
        {};

        template <std::size_t V>
        struct static_max<V>
          : std::integral_constant<std::size_t, V>
        {};

        template <std::size_t V0, std::size_t V1, std::size_t... Vs>
        struct static_max<V0, V1, Vs...>
          : static_max<(V0 < V1 ? V1 : V0), Vs...>
        {};
    }

    ///////////////////////////////////////////////////////////////////////////
    template <
        std::size_t Len,
        std::size_t Align = alignof(std::max_align_t),
        bool Copyable = false>
    class storage
    {
        union
        {
            alignas(Align) detail::byte _buffer[Len];
            detail::conditionally_copyable<Copyable> _copyable;
        };

    public:
        storage() = default;
        storage(storage const&) = default;
        storage& operator=(storage const&) = default;

        template <typename T, typename ...Args>
        T* construct(Args&&... args) noexcept(
            std::is_nothrow_constructible<T, Args...>::value)
        {
            return ::new (target()) T(std::forward<Args>(args)...);
        }
        template <typename T, typename U, typename ...Args>
        T* construct(std::initializer_list<U> il, Args&&... args) noexcept(
            std::is_nothrow_constructible<T, std::initializer_list<U>, Args...>::value)
        {
            return construct(il, std::forward<Args>(args)...);
        }

        template <typename T>
        void destroy() noexcept(
            std::is_nothrow_destructible<T>::value)
        {
            target<T>()->~T();
        }

        EGGS_CXX14_CONSTEXPR void* target() noexcept
        {
            return static_cast<void*>(&_buffer[0]);
        }
        constexpr void const* target() const noexcept
        {
            return static_cast<void const*>(&_buffer[0]);
        }

        template <typename T>
        EGGS_CXX14_CONSTEXPR T* target() noexcept
        {
            return detail::launder(static_cast<T*>(target()));
        }
        template <typename T>
        constexpr T const* target() const noexcept
        {
            return detail::launder(static_cast<T const*>(target()));
        }
    };

    template <typename ...Ts>
    using storage_for = storage<
        detail::static_max<sizeof(Ts)...>::value,
        detail::static_max<alignof(Ts)...>::value,
        /*Copyable=*/false>;

    template <typename ...Ts>
    using copyable_storage_for = storage<
        detail::static_max<sizeof(Ts)...>::value,
        detail::static_max<alignof(Ts)...>::value,
        /*Copyable=*/true>;

    ///////////////////////////////////////////////////////////////////////////
    template <std::size_t Len, std::size_t Align = alignof(std::max_align_t)>
    struct aligned_storage
    {
        using type = storage<
            Len, Align, /*Copyable=*/true>;
    };

    template <std::size_t Len, std::size_t Align = alignof(std::max_align_t)>
    using aligned_storage_t = typename aligned_storage<Len, Align>::type;

    ///////////////////////////////////////////////////////////////////////////
    template <std::size_t Len, typename... Ts>
    struct aligned_union {
      static_assert(sizeof...(Ts) > 0, "At least one type shall be provided.");

      static constexpr std::size_t alignment_value =
          detail::static_max<alignof(Ts)...>::value;

      using type = storage<
          detail::static_max<Len, sizeof(Ts)...>::value,
          alignment_value,
          /*Copyable=*/true>;
    };

    template <std::size_t Len, typename... Ts>
    using aligned_union_t = typename aligned_union<Len, Ts...>::type;
}}

/// constexpr support
#undef EGGS_CXX14_CONSTEXPR

#endif /*HPX_UTIL_STORAGE_HPP*/
