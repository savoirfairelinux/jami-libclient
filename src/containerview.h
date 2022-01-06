/*
 * Copyright (C) 2020-2022 by Savoir-faire Linux
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "typedefs.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <optional>
#include <type_traits>

template<class T>
using OptRef = typename std::optional<std::reference_wrapper<T>>;

// Some SFINAE helpers to clarify what happened when someone tries
// to make a ContainerView<float> or something wacky like that.
namespace detail {
template<typename... Ts>
struct has_defs
{};
template<typename T, typename _ = void>
struct is_container : std::false_type
{};
template<typename T>
struct is_container<T,
                    std::conditional_t<false,
                                       has_defs<decltype(std::declval<T>().front()),
                                                decltype(std::declval<T>().begin()),
                                                decltype(std::declval<T>().end())>,
                                       void>> : public std::true_type
{};
} // namespace detail

// Extra compile-time check to clarify why compilation has failed.
// Otherwise, the error message will be "'x' uses undefined struct ContainerView<decltype(x), void>"
template<class BaseType, class Enable = void>
struct ContainerView
{
    static_assert(detail::is_container<BaseType>::value == true,
                  "Template parameter is probably not a container!");
};

template<class BaseType>
struct ContainerView<BaseType, std::enable_if_t<detail::is_container<BaseType>::value>>
{
private:
    // Form a type by rebinding the underlying container type to a reference
    // wrapped value type.
    template<class ContainerType, class NewType>
    struct rebind;

    template<class ValueType, class... Args, template<class...> class ContainerType, class NewType>
    struct rebind<ContainerType<ValueType, Args...>, NewType>
    {
        using type = ContainerType<NewType, typename rebind<Args, NewType>::type...>;
    };

    using value_type = std::remove_reference_t<decltype(std::declval<BaseType>().front())>;
    using const_reference = const value_type&;
    using view_type = typename rebind<BaseType, std::reference_wrapper<value_type>>::type;

public:
    using FilterCallback = std::function<bool(const_reference)>;
    using SortCallback = std::function<bool(const_reference, const_reference)>;
    using OnEntryCallback = std::function<void(const_reference)>;

    ContainerView() = default;
    ContainerView(const BaseType& container)
    {
        data_ = std::make_optional(std::ref(container));
        auto& dataSource = std::remove_const_t<BaseType&>(data());
        view_.assign(dataSource.begin(), dataSource.end());
    };
    ContainerView(const ContainerView& other)
        : data_(std::nullopt)
        , view_(other().begin(), other().end())
        , dirty_(other.dirty_)
        , sortCallback_(other.sortCallback_)
        , filterCallback_(other.filterCallback_) {};
    ContainerView& operator=(const ContainerView& other) = default;
    ContainerView& operator=(ContainerView&& other) = default;
    ContainerView(ContainerView&& other) noexcept = default;

    // Allow concatenation of views.
    ContainerView& operator+=(const ContainerView& rhs)
    {
        view_.insert(view_.cend(), rhs.get().cbegin(), rhs.get().cend());
        return *this;
    }
    friend ContainerView operator+(ContainerView lhs, const ContainerView& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    // Reset the underlying container and initialize the view.
    ContainerView& reset(const BaseType& container)
    {
        data_ = std::make_optional(std::ref(container));
        auto& dataSource = std::remove_const_t<BaseType&>(data());
        view_.assign(dataSource.begin(), dataSource.end());
        invalidate();
        return *this;
    }

    // Alternately, reset the view to another view and disregard underlying data.
    ContainerView& reset(const ContainerView& other)
    {
        data_ = std::nullopt;
        auto& dataSource = std::remove_const_t<ContainerView&>(other);
        view_.assign(dataSource.get().begin(), dataSource.get().end());
        invalidate();
        return *this;
    }

    // Sort the reference wrapped elements of the view container with the
    // given predicate or the stored one.
    ContainerView& sort(SortCallback&& pred = {})
    {
        if (!dirty_) {
            std::cout << "view not dirty, no-op sort" << std::endl;
            return *this;
        }
        if (auto&& sortCallback = pred ? pred : sortCallback_)
            std::sort(view_.begin(), view_.end(), sortCallback);
        else
            std::cout << "no sort function specified or bound" << std::endl;
        return *this;
    }

    // Filter the reference wrapped elements of the view container with the
    // given predicate or the stored one.
    // Only done if the view has been invalidated(e.g. the underlying container
    // has been updated)
    ContainerView& filter(FilterCallback&& pred = {})
    {
        if (!dirty_) {
            std::cout << "view not dirty, no-op filter" << std::endl;
            return *this;
        }
        if (auto&& filterCallback = pred ? pred : filterCallback_) {
            if (data_.has_value()) {
                auto& dataSource = std::remove_const_t<BaseType&>(data());
                applyFilter(dataSource, filterCallback);
            } else {
                auto viewSource = view_;
                applyFilter(viewSource, filterCallback);
            }
        } else
            std::cout << "no filter function specified or bound" << std::endl;
        return *this;
    }

    // Iterate over the the reference wrapped elements of the view container
    // and execute a callback on each element.
    ContainerView& for_each(OnEntryCallback&& pred)
    {
        for (const auto& e : view_)
            pred(e);
        return *this;
    }

    // Store a non-static member function as a SortCallback.
    // The member function must match that of a binary predicate.
    template<typename T, typename... Args>
    void bindSortCallback(T* inst, bool (T::*func)(Args...))
    {
        bindCallback(sortCallback_, inst, func);
    }
    // Overload for function objects.
    template<typename Func = SortCallback>
    void bindSortCallback(Func&& func)
    {
        sortCallback_ = func;
    }

    // Store a non-static member function as a FilterCallback.
    // The member function must match that of a unary predicate.
    template<typename T, typename... Args>
    void bindFilterCallback(T* inst, bool (T::*func)(Args...))
    {
        bindCallback(filterCallback_, inst, func);
    }
    // Overload for function objects.
    template<typename Func = FilterCallback>
    void bindFilterCallback(Func&& func)
    {
        filterCallback_ = func;
    }

    // Basic container operations should be avoided.
    size_t size() const { return view_.size(); }
    const_reference at(size_t pos) const { return view_.at(pos); }
    void clear() { view_.clear(); }

    // TODO: re-filtering ?? should maybe observe underlying data in order to
    // not track this manually.
    bool isDirty() const noexcept { return dirty_; };
    ContainerView& invalidate() noexcept
    {
        dirty_ = true;
        return *this;
    };
    ContainerView& validate() noexcept
    {
        dirty_ = false;
        return *this;
    };

    // Returns whether or not this view has a concrete data source
    // or is just a view of a view.
    constexpr bool hasUnderlyingData() const noexcept { return data_.has_value(); }

    // Access the view.
    constexpr const view_type& operator()() const noexcept { return view_; }
    constexpr const view_type& get() const noexcept { return view_; }

private:
    // A reference to the optional underlying container data source.
    // If not used, the view can be constructed from another proxy
    // container view, and refiltered and sorted.
    OptRef<const BaseType> data_;

    // The 'view' is a container of reference wrapped values suitable
    // for container operations like sorting and filtering.
    view_type view_;

    // TODO: remove this invalidation flag if possible.
    bool dirty_ {true};

    // Stores the sorting/filtering predicates that can be re-applied
    // instead of passing a lambda as a parameter to sort().
    FilterCallback filterCallback_ {};

    // Same as above but for sorting.
    SortCallback sortCallback_ {};

    // A generic non-static member function storing function.
    template<typename C, typename T, typename... Args>
    void bindCallback(C& callback, T* inst, bool (T::*func)(Args...))
    {
        // Using a lambda instead of std::bind works better for functions with an
        // unknown number of parameters. e.g. callback = std::bind(func, inst, _1, ???);
        callback = [=](Args... args) -> bool {
            return (inst->*func)(args...);
        };
    }

    // Hard unbox and unwrap underlying data container.
    const BaseType& data() {
        //this fix error when build for macOS 10.13(error: 'value' is unavailable: introduced in macOS 10.14)
    #ifdef __APPLE__
        if(!data_.has_value())
           throw std::logic_error("bad optional access");
        return *data_;
    #else
        return data_.value().get();
    #endif
    }

    // Actually filter the view.
    template<typename T, typename Func = FilterCallback>
    ContainerView& applyFilter(T& source, Func&& pred)
    {
        view_.clear();
        std::copy_if(source.begin(), source.end(), std::back_inserter(view_), pred);
        return *this;
    }
};
