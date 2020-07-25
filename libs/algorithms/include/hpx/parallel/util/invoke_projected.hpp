//  Copyright (c) 2016 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>
#include <hpx/functional/invoke.hpp>
#include <hpx/type_support/decay.hpp>

#include <utility>

namespace hpx { namespace parallel { namespace util {
    ///////////////////////////////////////////////////////////////////////////
    template <typename Pred, typename Proj>
    struct invoke_projected
    {
        typedef typename hpx::util::decay<Pred>::type pred_type;
        typedef typename hpx::util::decay<Proj>::type proj_type;

        template <typename Pred_, typename Proj_>
        invoke_projected(Pred_&& pred, Proj_&& proj)
          : pred_(HPX_FWD(pred))
          , proj_(HPX_FWD(proj))
        {
        }

        template <typename T>
        auto operator()(
            T&& t) -> decltype(hpx::util::invoke(std::declval<pred_type>(),
            hpx::util::invoke(std::declval<proj_type>(), HPX_FWD(t))))
        {
            return hpx::util::invoke(
                pred_, hpx::util::invoke(proj_, HPX_FWD(t)));
        }

        pred_type pred_;
        proj_type proj_;
    };
}}}    // namespace hpx::parallel::util
