//  Copyright (c) 2007-2015 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>

// IWYU pragma: begin_exports
#include <hpx/async_local/dataflow.hpp>

#if defined(HPX_HAVE_DISTRIBUTED_RUNTIME)
#include <hpx/async_distributed/dataflow.hpp>
#endif
// IWYU pragma: end_exports
