//  Copyright (c) 2016-2017 Hartmut Kaiser
//  Copyright (c) 2018 Christopher Ogle
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>
#include <hpx/functional/invoke.hpp>

#include <utility>

namespace hpx { namespace parallel { namespace util {
    ///////////////////////////////////////////////////////////////////////////
    template <typename Compare, typename... Proj>
    struct compare_projected;

    template <typename Compare, typename Proj>
    struct compare_projected<Compare, Proj>
    {
        template <typename Compare_, typename Proj_>
        compare_projected(Compare_&& comp, Proj_&& proj)
          : comp_(HPX_FWD(comp))
          , proj_(HPX_FWD(proj))
        {
        }

        template <typename T1, typename T2>
        inline bool operator()(T1&& t1, T2&& t2) const
        {
            return hpx::util::invoke(comp_,
                hpx::util::invoke(proj_, HPX_FWD(t1)),
                hpx::util::invoke(proj_, HPX_FWD(t2)));
        }

        Compare comp_;
        Proj proj_;
    };

    template <typename Compare, typename Proj1, typename Proj2>
    struct compare_projected<Compare, Proj1, Proj2>
    {
        template <typename Compare_, typename Proj1_, typename Proj2_>
        compare_projected(Compare_&& comp, Proj1_&& proj1, Proj2_&& proj2)
          : comp_(HPX_FWD(comp))
          , proj1_(HPX_FWD(proj1))
          , proj2_(HPX_FWD(proj2))
        {
        }

        template <typename T1, typename T2>
        inline bool operator()(T1&& t1, T2&& t2) const
        {
            return hpx::util::invoke(comp_,
                hpx::util::invoke(proj1_, HPX_FWD(t1)),
                hpx::util::invoke(proj2_, HPX_FWD(t2)));
        }

        Compare comp_;
        Proj1 proj1_;
        Proj2 proj2_;
    };
}}}    // namespace hpx::parallel::util
