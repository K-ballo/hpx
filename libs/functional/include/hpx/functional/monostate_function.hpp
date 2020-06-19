//  Copyright (c) 2020 Agustin Berge
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>
#include <hpx/functional/invoke.hpp>
#include <hpx/functional/invoke_result.hpp>
#include <hpx/type_support/void_guard.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace hpx { namespace util {

    template <typename F, F Value>
    struct monostate_function_type
    {
        using value_type = F;
        static constexpr F value = Value;

        template <typename... Args>
        constexpr typename util::invoke_result<F const&, Args&&...>::type
        operator()(Args&&... args) const
        {
            return HPX_INVOKE(Value, std::forward<Args>(args)...);
        }
    };

    template <typename F, F Value>
    F const monostate_function_type<F, Value>::value;

    template <typename F, F Value>
    constexpr bool operator==(monostate_function_type<F, Value>,
        monostate_function_type<F, Value>) noexcept
    {
        return true;
    }

    template <typename F, F Value>
    constexpr bool operator!=(monostate_function_type<F, Value>,
        monostate_function_type<F, Value>) noexcept
    {
        return false;
    }

#if __cpp_nontype_template_parameter_auto
    template <auto Value>
    constexpr monostate_function_type<decltype(Value), Value>
        monostate_function = Value;
#endif

#define HPX_MONOSTATE_FUNCTION(...)                                            \
    ::hpx::util::monostate_function_type<decltype(__VA_ARGS__), (__VA_ARGS__)> \
    {                                                                          \
    }

}}    // namespace hpx::util
