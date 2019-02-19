//  Copyright (c) 2019 Agustin Berge
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HPX_UTIL_INTRUSIVE_PTR_HPP
#define HPX_UTIL_INTRUSIVE_PTR_HPP

#include <hpx/config.hpp>
#include <hpx/util/assert.hpp>

#include <cstddef>
#include <functional>
#include <type_traits>

namespace hpx { namespace util
{
    namespace detail
    {
        void intrusive_ptr_add_ref(void*) = delete;
        void intrusive_ptr_release(void*) = delete;
    }

    template <typename T>
    class intrusive_ptr
    {
        template <typename U>
        friend class intrusive_ptr;

    public:
        using element_type = T;

        // ctors
        HPX_CONSTEXPR intrusive_ptr() noexcept
          : _ptr(nullptr)
        {}

        HPX_CONSTEXPR intrusive_ptr(T* p, bool add_ref = true)
          : _ptr(p)
        {
            if (add_ref)
                this->add_ref();
        }

        template <typename U, typename Enable = typename std::enable_if<
                std::is_convertible<U*, T*>::value>::type>
        HPX_CONSTEXPR intrusive_ptr(U* p, bool add_ref = true)
          : _ptr(p)
        {
            if (add_ref)
                this->add_ref();
        }

        intrusive_ptr(intrusive_ptr const& other)
          : _ptr(other._ptr)
        {
            this->add_ref();
        }

        template <typename U, typename Enable = typename std::enable_if<
                std::is_convertible<U*, T*>::value>::type>
        intrusive_ptr(intrusive_ptr<U> const& other)
          : _ptr(other._ptr)
        {
            this->add_ref();
        }

        intrusive_ptr(intrusive_ptr&& other) noexcept
          : _ptr(other._ptr)
        {
            other._ptr = nullptr;
        }

        template <typename U, typename Enable = typename std::enable_if<
                std::is_convertible<U*, T*>::value>::type>
        intrusive_ptr(intrusive_ptr<U>&& other) noexcept
          : _ptr(other._ptr)
        {
            other._ptr = nullptr;
        }

        intrusive_ptr& operator=(intrusive_ptr const& other)
        {
            if (this != &other)
            {
                this->release();

                _ptr = other._ptr;
                this->add_ref();
            }
            return *this;
        }

        intrusive_ptr& operator=(intrusive_ptr&& other) noexcept
        {
            this->release();

            _ptr = other._ptr;
            other._ptr = nullptr;
            return *this;
        }

        ~intrusive_ptr()
        {
            this->release();
        }

        T* detach() noexcept
        {
            T* p = _ptr;
            _ptr = nullptr;
            return p;
        }

        // obs
        T& operator*() const noexcept
        {
            HPX_ASSERT(_ptr != nullptr);
            return *_ptr;
        }

        T* operator->() const noexcept
        {
            HPX_ASSERT(_ptr != nullptr);
            return _ptr;
        }

        T* get() const noexcept
        {
            return _ptr;
        }

        explicit operator bool() const noexcept
        {
            return _ptr != nullptr;
        }

        // ops
        friend HPX_CONSTEXPR bool operator==(
            intrusive_ptr<T> const& lhs,
            intrusive_ptr<T> const& rhs) noexcept
        {
            return lhs._ptr == rhs._ptr;
        }

        friend HPX_CONSTEXPR bool operator!=(
            intrusive_ptr<T> const& lhs,
            intrusive_ptr<T> const& rhs) noexcept
        {
            return !(lhs == rhs);
        }

        friend HPX_CONSTEXPR bool operator<(
            intrusive_ptr<T> const& lhs,
            intrusive_ptr<T> const& rhs) noexcept
        {
            return std::less<T*>{}(lhs, rhs);
        }

        friend HPX_CONSTEXPR bool operator>(
            intrusive_ptr<T> const& lhs,
            intrusive_ptr<T> const& rhs) noexcept
        {
            return rhs < lhs;
        }

        friend HPX_CONSTEXPR bool operator<=(
            intrusive_ptr<T> const& lhs,
            intrusive_ptr<T> const& rhs) noexcept
        {
            return !(rhs < lhs);
        }

        friend HPX_CONSTEXPR bool operator>=(
            intrusive_ptr<T> const& lhs,
            intrusive_ptr<T> const& rhs) noexcept
        {
            return !(lhs < rhs);
        }

    private:
        void add_ref()
        {
            using detail::intrusive_ptr_add_ref;
            if (_ptr != nullptr)
                intrusive_ptr_add_ref(_ptr);
        }

        void release()
        {
            using detail::intrusive_ptr_release;
            if (_ptr != nullptr)
                intrusive_ptr_release(_ptr);
        }

    private:
        T* _ptr;
    };

    template <typename T, typename U>
    static HPX_CONSTEXPR intrusive_ptr<T> static_pointer_cast(
        intrusive_ptr<U> const& ptr, bool add_ref = true) noexcept
    {
        return intrusive_ptr<T>{static_cast<T*>(ptr.get()), add_ref};
    };
}}

namespace std
{
    template <typename T>
    struct hash< ::hpx::util::intrusive_ptr<T>>
    {
        HPX_CONSTEXPR std::size_t operator()(
            ::hpx::util::intrusive_ptr<T> const& ptr) const noexcept
        {
            return std::hash<T*>{}(ptr.get());
        }
    };
}

///////////////////////////////////////////////////////////////////////////////
#include <hpx/runtime/serialization/detail/pointer.hpp>

namespace hpx { namespace serialization
{
    template <typename T>
    void load(input_archive& ar, util::intrusive_ptr<T>& ptr, unsigned)
    {
        detail::serialize_pointer_tracked(ar, ptr);
    }

    template <typename T>
    void save(output_archive& ar, util::intrusive_ptr<T> const& ptr, unsigned)
    {
        detail::serialize_pointer_tracked(ar, ptr);
    }

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE((template <class T>),
            (util::intrusive_ptr<T>));
}}

#endif /*HPX_UTIL_INTRUSIVE_PTR_HPP*/
