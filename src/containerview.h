/*
 * Copyright (C) 2020 by Savoir-faire Linux
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

template<class Container>
struct ContainerView
{
private:
    template<class Container, class NewType>
    struct rebind;

    template<class ValueType, class... Args, template<class...> class Container, class NewType>
    struct rebind<Container<ValueType, Args...>, NewType>
    {
        typedef Container<NewType, typename rebind<Args, NewType>::type...> type;
    };

    using value_t = std::remove_reference_t<decltype(std::declval<Container>().front())>;
    using ref_container_t = typename rebind<Container, std::reference_wrapper<value_t>>::type;
    using const_reference_t = const value_t&;

public:
    using FilterPredicate = std::function<bool(const_reference_t)>;
    using SortPredicate = std::function<bool(const_reference_t, const_reference_t)>;
    using OnEntryCallback = std::function<void(const_reference_t)>;

    ContainerView() {};
    ContainerView(Container& container)
        : source_(container.begin(), container.end())
        , view_(source_) {};
    ContainerView(const ContainerView& other)
        : source_(other.view_)
        , view_(source_) {};

    ContainerView& reset(Container& container)
    {
        source_.assign(container.begin(), container.end());
        view_.assign(source_.begin(), source_.end());
        dirty = false;
        return *this;
    }

    template<typename F = FilterPredicate>
    ContainerView& filter(F&& pred)
    {
        view_.clear();
        std::copy_if(source_.begin(), source_.end(), std::back_inserter(view_), pred);
        return *this;
    }

    template<typename F = SortPredicate>
    ContainerView& sort(F&& pred)
    {
        std::sort(view_.begin(), view_.end(), pred);
        return *this;
    }

    template<typename F = OnEntryCallback>
    void for_each(F&& pred)
    {
        for (const auto& a : view_)
            pred(a);
    }

    ref_container_t operator()() { return view_; }
    size_t size() const { return view_.size(); }
    const_reference_t at(size_t pos) const { return view_.at(pos); }
    void clear() { view_.clear(); }

    bool dirty {true};

private:
    ref_container_t source_;
    ref_container_t view_;
};
