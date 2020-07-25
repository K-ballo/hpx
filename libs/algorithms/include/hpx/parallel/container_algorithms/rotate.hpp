//  Copyright (c) 2007-2015 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// \file parallel/container_algorithms/rotate.hpp

#pragma once

#include <hpx/config.hpp>
#include <hpx/concepts/concepts.hpp>
#include <hpx/iterator_support/range.hpp>
#include <hpx/iterator_support/traits/is_iterator.hpp>
#include <hpx/iterator_support/traits/is_range.hpp>
#include <hpx/parallel/util/tagged_pair.hpp>

#include <hpx/algorithms/traits/projected_range.hpp>
#include <hpx/parallel/algorithms/rotate.hpp>
#include <hpx/parallel/tagspec.hpp>
#include <hpx/parallel/util/projection_identity.hpp>
#include <hpx/parallel/util/result_types.hpp>

#include <type_traits>
#include <utility>

namespace hpx { namespace parallel { inline namespace v1 {
    ///////////////////////////////////////////////////////////////////////////
    /// Performs a left rotation on a range of elements. Specifically,
    /// \a rotate swaps the elements in the range [first, last) in such a way
    /// that the element new_first becomes the first element of the new range
    /// and new_first - 1 becomes the last element.
    ///
    /// \note   Complexity: Linear in the distance between \a first and \a last.
    ///
    /// \tparam ExPolicy    The type of the execution policy to use (deduced).
    ///                     It describes the manner in which the execution
    ///                     of the algorithm may be parallelized and the manner
    ///                     in which it executes the assignments.
    /// \tparam Rng         The type of the source range used (deduced).
    ///                     The iterators extracted from this range type must
    ///                     meet the requirements of a forward iterator.
    ///
    /// \param policy       The execution policy to use for the scheduling of
    ///                     the iterations.
    /// \param rng          Refers to the sequence of elements the algorithm
    ///                     will be applied to.
    /// \param middle       Refers to the element that should appear at the
    ///                     beginning of the rotated range.
    ///
    /// The assignments in the parallel \a rotate algorithm invoked
    /// with an execution policy object of type \a sequenced_policy
    /// execute in sequential order in the calling thread.
    ///
    /// The assignments in the parallel \a rotate algorithm invoked with
    /// an execution policy object of type \a parallel_policy or
    /// \a parallel_task_policy are permitted to execute in an unordered
    /// fashion in unspecified threads, and indeterminately sequenced
    /// within each thread.
    ///
    /// \note The type of dereferenced \a FwdIter must meet the requirements
    ///       of \a MoveAssignable and \a MoveConstructible.
    ///
    /// \returns  The \a rotate algorithm returns a
    ///           \a hpx::future<tagged_pair<tag::begin(FwdIter), tag::end(FwdIter)> >
    ///           if the execution policy is of type
    ///           \a parallel_task_policy and
    ///           returns \a tagged_pair<tag::begin(FwdIter), tag::end(FwdIter)>
    ///           otherwise.
    ///           The \a rotate algorithm returns the iterator equal to
    ///           pair(first + (last - new_first), last).
    ///
    template <typename ExPolicy, typename Rng,
        HPX_CONCEPT_REQUIRES_(execution::is_execution_policy<ExPolicy>::value&&
                hpx::traits::is_range<Rng>::value)>
    typename util::detail::algorithm_result<ExPolicy,
        util::in_out_result<typename hpx::traits::range_iterator<Rng>::type,
            typename hpx::traits::range_iterator<Rng>::type>>::type
    rotate(ExPolicy&& policy, Rng&& rng,
        typename hpx::traits::range_iterator<Rng>::type middle)
    {
        return rotate(HPX_FWD(policy), hpx::util::begin(rng),
            middle, hpx::util::end(rng));
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Copies the elements from the range [first, last), to another range
    /// beginning at \a dest_first in such a way, that the element
    /// \a new_first becomes the first element of the new range and
    /// \a new_first - 1 becomes the last element.
    ///
    /// \note   Complexity: Performs exactly \a last - \a first assignments.
    ///
    /// \tparam ExPolicy    The type of the execution policy to use (deduced).
    ///                     It describes the manner in which the execution
    ///                     of the algorithm may be parallelized and the manner
    ///                     in which it executes the assignments.
    /// \tparam Rng         The type of the source range used (deduced).
    ///                     The iterators extracted from this range type must
    ///                     meet the requirements of a forward iterator.
    /// \tparam OutIter     The type of the iterator representing the
    ///                     destination range (deduced).
    ///                     This iterator type must meet the requirements of an
    ///                     output iterator.
    ///
    /// \param policy       The execution policy to use for the scheduling of
    ///                     the iterations.
    /// \param rng          Refers to the sequence of elements the algorithm
    ///                     will be applied to.
    /// \param middle       Refers to the element that should appear at the
    ///                     beginning of the rotated range.
    /// \param dest_first   Refers to the begin of the destination range.
    ///
    /// The assignments in the parallel \a rotate_copy algorithm invoked
    /// with an execution policy object of type \a sequenced_policy
    /// execute in sequential order in the calling thread.
    ///
    /// The assignments in the parallel \a rotate_copy algorithm invoked with
    /// an execution policy object of type \a parallel_policy or
    /// \a parallel_task_policy are permitted to execute in an unordered
    /// fashion in unspecified threads, and indeterminately sequenced
    /// within each thread.
    ///
    /// \returns  The \a rotate_copy algorithm returns a
    ///           \a hpx::future<tagged_pair<tag::in(FwdIter), tag::out(OutIter)> >
    ///           if the execution policy is of type
    ///           \a parallel_task_policy and
    ///           returns \a tagged_pair<tag::in(FwdIter), tag::out(OutIter)>
    ///           otherwise.
    ///           The \a rotate_copy algorithm returns the output iterator to the
    ///           element past the last element copied.
    ///
    template <typename ExPolicy, typename Rng, typename OutIter,
        HPX_CONCEPT_REQUIRES_(execution::is_execution_policy<ExPolicy>::value&&
                hpx::traits::is_range<Rng>::value&&
                    hpx::traits::is_iterator<OutIter>::value)>
    typename util::detail::algorithm_result<ExPolicy,
        util::in_out_result<typename hpx::traits::range_iterator<Rng>::type,
            OutIter>>::type
    rotate_copy(ExPolicy&& policy, Rng&& rng,
        typename hpx::traits::range_iterator<Rng>::type middle,
        OutIter dest_first)
    {
        return rotate_copy(HPX_FWD(policy),
            hpx::util::begin(rng), middle, hpx::util::end(rng), dest_first);
    }
}}}    // namespace hpx::parallel::v1
