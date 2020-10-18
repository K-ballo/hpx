//  Copyright (c) 2007-2012 Hartmut Kaiser
//  Copyright (c)      2011 Bryce Lelbach
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>

// IWYU pragma: begin_exports
#include <hpx/include/resource_partitioner.hpp>
#include <hpx/include/threadmanager.hpp>
#include <hpx/runtime_local/runtime_local.hpp>

#if defined(HPX_HAVE_DISTRIBUTED_RUNTIME)
#include <hpx/include/actions.hpp>
#include <hpx/include/applier.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/parcelset.hpp>
#include <hpx/runtime_distributed.hpp>
#endif
// IWYU pragma: end_exports
