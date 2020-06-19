//  Copyright (c) 2016 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/runtime_local/run_as_os_thread.hpp>

#include <hpx/components/process/child.hpp>
#include <hpx/components/process/server/child.hpp>

#include <hpx/components/process/util/terminate.hpp>
#include <hpx/components/process/util/wait_for_exit.hpp>

#include <functional>

///////////////////////////////////////////////////////////////////////////////
HPX_REGISTER_ACTION(hpx::components::process::server::child::terminate_action,
    hpx_components_process_server_child_terminate_action);

HPX_REGISTER_ACTION(
    hpx::components::process::server::child::wait_for_exit_action,
    hpx_components_process_server_child_wait_for_exit);

namespace hpx { namespace components { namespace process { namespace server {
    void child::terminate()
    {
        process::util::terminate(child_);
    }

    int child::wait_for_exit()
    {
        return hpx::threads::run_as_os_thread(
            HPX_MONOSTATE_FUNCTION(
                static_cast<int (*)(process::util::child const&)>(
                    &process::util::wait_for_exit<process::util::child>)),
            std::ref(child_))
            .get();
    }
}}}}    // namespace hpx::components::process::server
