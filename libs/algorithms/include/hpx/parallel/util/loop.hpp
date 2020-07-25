//  Copyright (c) 2007-2016 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>
#if defined(HPX_HAVE_DATAPAR)
#include <hpx/parallel/datapar/loop.hpp>
#endif
#include <hpx/assert.hpp>
#include <hpx/datastructures/tuple.hpp>
#include <hpx/execution/traits/is_execution_policy.hpp>
#include <hpx/functional/invoke.hpp>
#include <hpx/functional/invoke_result.hpp>
#include <hpx/parallel/util/cancellation_token.hpp>
#include <hpx/parallel/util/projection_identity.hpp>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

namespace hpx { namespace parallel { namespace util {
    ///////////////////////////////////////////////////////////////////////////
    template <typename ExPolicy, typename VecOnly, typename F,
        typename... Iters>
    HPX_HOST_DEVICE HPX_FORCEINLINE typename std::enable_if<
        !execution::is_vectorpack_execution_policy<ExPolicy>::value,
        typename hpx::util::invoke_result<F, Iters...>::type>::type
    loop_step(VecOnly, F&& f, Iters&... its)
    {
        return hpx::util::invoke(HPX_FWD(f), (its++)...);
    }

    template <typename ExPolicy, typename Iter>
    HPX_HOST_DEVICE HPX_FORCEINLINE constexpr typename std::enable_if<
        !execution::is_vectorpack_execution_policy<ExPolicy>::value, bool>::type
        loop_optimization(Iter, Iter)
    {
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {
        ///////////////////////////////////////////////////////////////////////
        // Helper class to repeatedly call a function starting from a given
        // iterator position.
        template <typename Iterator>
        struct loop
        {
            ///////////////////////////////////////////////////////////////////
            template <typename Begin, typename End, typename F>
            HPX_HOST_DEVICE HPX_FORCEINLINE static Begin call(
                Begin it, End end, F&& f)
            {
                for (/**/; it != end; ++it)
                    f(it);

                return it;
            }

            template <typename Begin, typename End, typename CancelToken,
                typename F>
            HPX_HOST_DEVICE HPX_FORCEINLINE static Begin call(
                Begin it, End end, CancelToken& tok, F&& f)
            {
                for (/**/; it != end; ++it)
                {
                    if (tok.was_cancelled())
                        break;
                    f(it);
                }
                return it;
            }
        };
    }    // namespace detail

    template <typename ExPolicy, typename Begin, typename End, typename F>
    HPX_HOST_DEVICE HPX_FORCEINLINE Begin loop(
        ExPolicy&&, Begin begin, End end, F&& f)
    {
        return detail::loop<Begin>::call(begin, end, HPX_FWD(f));
    }

    template <typename ExPolicy, typename Begin, typename End,
        typename CancelToken, typename F>
    HPX_HOST_DEVICE HPX_FORCEINLINE Begin loop(
        ExPolicy&&, Begin begin, End end, CancelToken& tok, F&& f)
    {
        return detail::loop<Begin>::call(begin, end, tok, HPX_FWD(f));
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {
        ///////////////////////////////////////////////////////////////////////
        // Helper class to repeatedly call a function starting from a given
        // iterator position.
        template <typename Iter1, typename Iter2>
        struct loop2
        {
            ///////////////////////////////////////////////////////////////////
            template <typename Begin1, typename End1, typename Begin2,
                typename F>
            HPX_HOST_DEVICE HPX_FORCEINLINE static std::pair<Begin1, Begin2>
            call(Begin1 it1, End1 end1, Begin2 it2, F&& f)
            {
                for (/**/; it1 != end1; (void) ++it1, ++it2)
                    f(it1, it2);

                return std::make_pair(std::move(it1), std::move(it2));
            }
        };
    }    // namespace detail

    template <typename ExPolicy, typename VecOnly, typename Begin1,
        typename End1, typename Begin2, typename F>
    HPX_HOST_DEVICE HPX_FORCEINLINE typename std::enable_if<
        !execution::is_vectorpack_execution_policy<ExPolicy>::value,
        std::pair<Begin1, Begin2>>::type
    loop2(VecOnly, Begin1 begin1, End1 end1, Begin2 begin2, F&& f)
    {
        return detail::loop2<Begin1, Begin2>::call(
            begin1, end1, begin2, HPX_FWD(f));
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {
        // Helper class to repeatedly call a function a given number of times
        // starting from a given iterator position.

        template <typename Iterator>
        struct loop_n
        {
            ///////////////////////////////////////////////////////////////////
            // handle sequences of non-futures
            template <typename Iter, typename F>
            HPX_HOST_DEVICE HPX_FORCEINLINE static Iter call(
                Iter it, std::size_t num, F&& f)
            {
                std::size_t count(num & std::size_t(-4));    // -V112
                for (std::size_t i = 0; i < count;
                     (void) ++it, i += 4)    // -V112
                {
                    f(it);
                    f(++it);
                    f(++it);
                    f(++it);
                }
                for (/**/; count < num; (void) ++count, ++it)
                {
                    f(it);
                }
                return it;
            }

            template <typename Iter, typename CancelToken, typename F>
            HPX_HOST_DEVICE HPX_FORCEINLINE static Iter call(
                Iter it, std::size_t num, CancelToken& tok, F&& f)
            {
                std::size_t count(num & std::size_t(-4));    // -V112
                for (std::size_t i = 0; i < count;
                     (void) ++it, i += 4)    // -V112
                {
                    if (tok.was_cancelled())
                        break;
                    f(it);
                    f(++it);
                    f(++it);
                    f(++it);
                }
                for (/**/; count < num; (void) ++count, ++it)
                {
                    if (tok.was_cancelled())
                        break;
                    f(it);
                }
                return it;
            }
        };

        ///////////////////////////////////////////////////////////////////////
        template <typename ExPolicy, typename T>
        HPX_HOST_DEVICE HPX_FORCEINLINE typename std::enable_if<
            !execution::is_vectorpack_execution_policy<ExPolicy>::value,
            T const&>::type
        extract_value(T const& v)
        {
            return v;
        }

        template <typename ExPolicy, typename F, typename T>
        HPX_HOST_DEVICE HPX_FORCEINLINE typename std::enable_if<
            !execution::is_vectorpack_execution_policy<ExPolicy>::value,
            T const&>::type
        accumulate_values(F&&, T const& v)
        {
            return v;
        }

        template <typename ExPolicy, typename F, typename T, typename T1>
        HPX_HOST_DEVICE HPX_FORCEINLINE typename std::enable_if<
            !execution::is_vectorpack_execution_policy<ExPolicy>::value,
            T>::type
        accumulate_values(F&& f, T&& v, T1&& init)
        {
            return hpx::util::invoke(
                HPX_FWD(f), HPX_FWD(init), HPX_FWD(v));
        }
    }    // namespace detail

    template <typename ExPolicy, typename Iter, typename F>
    HPX_HOST_DEVICE HPX_FORCEINLINE typename std::enable_if<
        !execution::is_vectorpack_execution_policy<ExPolicy>::value, Iter>::type
    loop_n(Iter it, std::size_t count, F&& f)
    {
        return detail::loop_n<Iter>::call(it, count, HPX_FWD(f));
    }

    template <typename ExPolicy, typename Iter, typename CancelToken,
        typename F>
    HPX_HOST_DEVICE HPX_FORCEINLINE typename std::enable_if<
        !execution::is_vectorpack_execution_policy<ExPolicy>::value, Iter>::type
    loop_n(Iter it, std::size_t count, CancelToken& tok, F&& f)
    {
        return detail::loop_n<Iter>::call(it, count, tok, HPX_FWD(f));
    }

    ///////////////////////////////////////////////////////////////////////////
    //     namespace detail
    //     {
    //         ///////////////////////////////////////////////////////////////////////
    //         // Helper class to repeatedly call a function starting from a given
    //         // iterator position.
    //         template <typename Iter1, typename Iter2>
    //         struct loop2_n
    //         {
    //             ///////////////////////////////////////////////////////////////////
    //             template <typename Begin1, typename Begin2, typename F>
    //             HPX_HOST_DEVICE HPX_FORCEINLINE
    //             static std::pair<Begin1, Begin2>
    //             call(Begin1 it1, std::size_t count, Begin2 it2, F && f)
    //             {
    //                 for (/**/; count != 0; (void) ++it1, ++it2, --count)
    //                     f(it1, it2);
    //
    //                 return std::make_pair(it1, it2);
    //             }
    //         };
    //     }
    //
    //     template <typename ExPolicy, typename Begin1, typename Begin2, typename F>
    //     HPX_HOST_DEVICE HPX_FORCEINLINE std::pair<Begin1, Begin2>
    //     loop2_n(ExPolicy&&, Begin1 begin1, std::size_t count, Begin2 begin2, F && f)
    //     {
    //         return detail::loop2_n<Begin1, Begin2>::call(begin1, count, begin2,
    //             HPX_FWD(f));
    //     }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {
        // Helper class to repeatedly call a function a given number of times
        // starting from a given iterator position. If an exception is thrown,
        // the given cleanup function will be called.
        template <typename IterCat>
        struct loop_with_cleanup
        {
            ///////////////////////////////////////////////////////////////////
            template <typename FwdIter, typename F, typename Cleanup>
            static FwdIter call(
                FwdIter it, FwdIter last, F&& f, Cleanup&& cleanup)
            {
                FwdIter base = it;
                try
                {
                    for (/**/; it != last; ++it)
                        f(it);
                    return it;
                }
                catch (...)
                {
                    for (/**/; base != it; ++base)
                        cleanup(base);
                    throw;
                }
            }

            template <typename Iter, typename FwdIter, typename F,
                typename Cleanup>
            static FwdIter call(
                Iter it, Iter last, FwdIter dest, F&& f, Cleanup&& cleanup)
            {
                FwdIter base = dest;
                try
                {
                    for (/**/; it != last; (void) ++it, ++dest)
                        f(it, dest);
                    return dest;
                }
                catch (...)
                {
                    for (/**/; base != dest; ++base)
                        cleanup(base);
                    throw;
                }
            }
        };
    }    // namespace detail

    ///////////////////////////////////////////////////////////////////////////
    template <typename Iter, typename F, typename Cleanup>
    HPX_FORCEINLINE Iter loop_with_cleanup(
        Iter it, Iter last, F&& f, Cleanup&& cleanup)
    {
        typedef typename std::iterator_traits<Iter>::iterator_category cat;
        return detail::loop_with_cleanup<cat>::call(
            it, last, HPX_FWD(f), HPX_FWD(cleanup));
    }

    template <typename Iter, typename FwdIter, typename F, typename Cleanup>
    HPX_FORCEINLINE FwdIter loop_with_cleanup(
        Iter it, Iter last, FwdIter dest, F&& f, Cleanup&& cleanup)
    {
        typedef typename std::iterator_traits<Iter>::iterator_category cat;
        return detail::loop_with_cleanup<cat>::call(
            it, last, dest, HPX_FWD(f), HPX_FWD(cleanup));
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {
        // Helper class to repeatedly call a function a given number of times
        // starting from a given iterator position.
        template <typename IterCat>
        struct loop_with_cleanup_n
        {
            ///////////////////////////////////////////////////////////////////
            template <typename FwdIter, typename F, typename Cleanup>
            static FwdIter call(
                FwdIter it, std::size_t count, F&& f, Cleanup&& cleanup)
            {
                FwdIter base = it;
                try
                {
                    for (/**/; count != 0; (void) --count, ++it)
                        f(it);
                    return it;
                }
                catch (...)
                {
                    for (/**/; base != it; ++base)
                        cleanup(base);
                    throw;
                }
            }

            template <typename Iter, typename FwdIter, typename F,
                typename Cleanup>
            static FwdIter call(Iter it, std::size_t count, FwdIter dest, F&& f,
                Cleanup&& cleanup)
            {
                FwdIter base = dest;
                try
                {
                    for (/**/; count != 0; (void) --count, ++it, ++dest)
                        f(it, dest);
                    return dest;
                }
                catch (...)
                {
                    for (/**/; base != dest; ++base)
                        cleanup(base);
                    throw;
                }
            }

            ///////////////////////////////////////////////////////////////////
            template <typename FwdIter, typename CancelToken, typename F,
                typename Cleanup>
            static FwdIter call_with_token(FwdIter it, std::size_t count,
                CancelToken& tok, F&& f, Cleanup&& cleanup)
            {
                FwdIter base = it;
                try
                {
                    for (/**/; count != 0; (void) --count, ++it)
                    {
                        if (tok.was_cancelled())
                            break;
                        f(it);
                    }
                    return it;
                }
                catch (...)
                {
                    tok.cancel();
                    for (/**/; base != it; ++base)
                        cleanup(base);
                    throw;
                }
            }

            template <typename Iter, typename FwdIter, typename CancelToken,
                typename F, typename Cleanup>
            static FwdIter call_with_token(Iter it, std::size_t count,
                FwdIter dest, CancelToken& tok, F&& f, Cleanup&& cleanup)
            {
                FwdIter base = dest;
                try
                {
                    for (/**/; count != 0; (void) --count, ++it, ++dest)
                    {
                        if (tok.was_cancelled())
                            break;
                        f(it, dest);
                    }
                    return dest;
                }
                catch (...)
                {
                    tok.cancel();
                    for (/**/; base != dest; ++base)
                        cleanup(base);
                    throw;
                }
            }
        };
    }    // namespace detail

    ///////////////////////////////////////////////////////////////////////////
    template <typename Iter, typename F, typename Cleanup>
    HPX_FORCEINLINE Iter loop_with_cleanup_n(
        Iter it, std::size_t count, F&& f, Cleanup&& cleanup)
    {
        typedef typename std::iterator_traits<Iter>::iterator_category cat;
        return detail::loop_with_cleanup_n<cat>::call(
            it, count, HPX_FWD(f), HPX_FWD(cleanup));
    }

    template <typename Iter, typename FwdIter, typename F, typename Cleanup>
    HPX_FORCEINLINE FwdIter loop_with_cleanup_n(
        Iter it, std::size_t count, FwdIter dest, F&& f, Cleanup&& cleanup)
    {
        typedef typename std::iterator_traits<Iter>::iterator_category cat;
        return detail::loop_with_cleanup_n<cat>::call(it, count, dest,
            HPX_FWD(f), HPX_FWD(cleanup));
    }

    template <typename Iter, typename CancelToken, typename F, typename Cleanup>
    HPX_FORCEINLINE Iter loop_with_cleanup_n_with_token(
        Iter it, std::size_t count, CancelToken& tok, F&& f, Cleanup&& cleanup)
    {
        typedef typename std::iterator_traits<Iter>::iterator_category cat;
        return detail::loop_with_cleanup_n<cat>::call_with_token(
            it, count, tok, HPX_FWD(f), HPX_FWD(cleanup));
    }

    template <typename Iter, typename FwdIter, typename CancelToken, typename F,
        typename Cleanup>
    HPX_FORCEINLINE FwdIter loop_with_cleanup_n_with_token(Iter it,
        std::size_t count, FwdIter dest, CancelToken& tok, F&& f,
        Cleanup&& cleanup)
    {
        typedef typename std::iterator_traits<Iter>::iterator_category cat;
        return detail::loop_with_cleanup_n<cat>::call_with_token(it, count,
            dest, tok, HPX_FWD(f), HPX_FWD(cleanup));
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {
        // Helper class to repeatedly call a function a given number of times
        // starting from a given iterator position.
        template <typename IterCat>
        struct loop_idx_n
        {
            ///////////////////////////////////////////////////////////////////
            // handle sequences of non-futures
            template <typename Iter, typename F>
            static Iter call(
                std::size_t base_idx, Iter it, std::size_t count, F&& f)
            {
                for (/**/; count != 0; (void) --count, ++it, ++base_idx)
                    f(*it, base_idx);

                return it;
            }

            template <typename Iter, typename CancelToken, typename F>
            static Iter call(std::size_t base_idx, Iter it, std::size_t count,
                CancelToken& tok, F&& f)
            {
                for (/**/; count != 0; (void) --count, ++it, ++base_idx)
                {
                    if (tok.was_cancelled(base_idx))
                        break;
                    f(*it, base_idx);
                }
                return it;
            }
        };
    }    // namespace detail

    ///////////////////////////////////////////////////////////////////////////
    template <typename Iter, typename F>
    HPX_FORCEINLINE Iter loop_idx_n(
        std::size_t base_idx, Iter it, std::size_t count, F&& f)
    {
        typedef typename std::iterator_traits<Iter>::iterator_category cat;
        return detail::loop_idx_n<cat>::call(
            base_idx, it, count, HPX_FWD(f));
    }

    template <typename Iter, typename CancelToken, typename F>
    HPX_FORCEINLINE Iter loop_idx_n(std::size_t base_idx, Iter it,
        std::size_t count, CancelToken& tok, F&& f)
    {
        typedef typename std::iterator_traits<Iter>::iterator_category cat;
        return detail::loop_idx_n<cat>::call(
            base_idx, it, count, tok, HPX_FWD(f));
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {
        // Helper class to repeatedly call a function a given number of times
        // starting from a given iterator position.
        template <typename IterCat>
        struct accumulate_n
        {
            template <typename Iter, typename T, typename Pred>
            static T call(Iter it, std::size_t count, T init, Pred&& f)
            {
                for (/**/; count != 0; (void) --count, ++it)
                    init = f(init, *it);
                return init;
            }
        };
    }    // namespace detail

    ///////////////////////////////////////////////////////////////////////////
    template <typename Iter, typename T, typename Pred>
    HPX_FORCEINLINE T accumulate_n(Iter it, std::size_t count, T init, Pred&& f)
    {
        typedef typename std::iterator_traits<Iter>::iterator_category cat;
        return detail::accumulate_n<cat>::call(
            it, count, std::move(init), HPX_FWD(f));
    }

    template <typename T, typename Iter, typename Reduce,
        typename Conv = util::projection_identity>
    HPX_FORCEINLINE T accumulate(
        Iter first, Iter last, Reduce&& r, Conv&& conv = Conv())
    {
        T val = hpx::util::invoke(conv, *first);
        ++first;
        while (last != first)
        {
            val = hpx::util::invoke(r, val, *first);
            ++first;
        }
        return val;
    }

    template <typename T, typename Iter1, typename Iter2, typename Reduce,
        typename Conv>
    HPX_FORCEINLINE T accumulate(
        Iter1 first1, Iter1 last1, Iter2 first2, Reduce&& r, Conv&& conv)
    {
        T val = hpx::util::invoke(conv, *first1, *first2);
        ++first1;
        ++first2;
        while (last1 != first1)
        {
            val = hpx::util::invoke(
                r, val, hpx::util::invoke(conv, *first1, *first2));
            ++first1;
            ++first2;
        }
        return val;
    }
}}}    // namespace hpx::parallel::util
