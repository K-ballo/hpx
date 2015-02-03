//  Copyright (c) 2007-2012 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HPX_UTIL_PP_DISPATCH_HPP
#define HPX_UTIL_PP_DISPATCH_HPP

#include <hpx/util/pp/dispatch.hpp>
#include <hpx/util/pp/va_nargs.hpp>

#include <boost/preprocessor/cat.hpp>

#define HPX_UTIL_PP_DISPATCH(I, ...)                                          \
    HPX_UTIL_PP_DISPATCH_E(HPX_UTIL_PP_DISPATCH_I(                            \
        BOOST_PP_CAT(I, HPX_UTIL_PP_VA_NARGS(__VA_ARGS__)), __VA_ARGS__))     \
/**/

#define HPX_UTIL_PP_DISPATCH_E(I)                                             \
    I                                                                         \
/**/

#define HPX_UTIL_PP_DISPATCH_I(I, ...)                                        \
    HPX_UTIL_PP_DISPATCH_E(I(__VA_ARGS__))                                    \
/**/

#endif
