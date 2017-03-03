/****************************************************************************
 *   Copyright (C) 2017 by Savoir-faire Linux                               *
 *   Author : Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>      *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Lesser General Public             *
 *   License as published by the Free Software Foundation; either           *
 *   version 2.1 of the License, or (at your option) any later version.     *
 *                                                                          *
 *   This library is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/

#pragma once

//Std
#include <time.h>

/// \brief UsageStatistics is a container to store common statistics about
/// usage of arbitrary object.
class UsageStatistics
{
public:
    //Accessors

    unsigned totalCount() const {
        return m_TotalSeconds;
    }

    unsigned totalSeconds() const {
        return m_TotalSeconds;
    }

    unsigned lastWeekCount() const {
        return m_LastWeekCount;
    }

    unsigned lastTrimCount() const {
        return m_LastTrimCount;
    }

    time_t lastUsed() const {
        return m_LastUsed;
    }

    bool haveCalled() const {
        return m_HaveCalled;
    }

    //Mutators

    void setHaveCalled() {
        m_HaveCalled = true;
    }

    /// \brief Update usage using a time range.
    ///
    /// All values are updated using given <tt>[start, stop]</tt> time range.
    /// \a start and \a stop are given in seconds.
    ///
    /// \param start starting time of usage
    /// \param stop ending time of usage, must be greater than \a start
    void update(const time_t& start, const time_t& stop) {
        ++m_TotalCount;
        setLastUsed(start);
        m_TotalSeconds += stop - start;
        time_t now;
        ::time(&now);
        if (now - 3600*24*7 < stop)
            ++m_LastWeekCount;
        if (now - 3600*24*7*15 < stop)
            ++m_LastTrimCount;
    }

    /// \brief Use this method to update lastUsed time by a new time only if sooner.
    ///
    /// \return \a true if the update has been effective.
    bool setLastUsed(time_t time) {
        if (m_LastUsed < time) {
            m_LastUsed = time;
            return true;
        }
        return false;
    }

    /// \brief Inplace incrementation of current values by values from another UsageStatistics instance.
    ///
    /// \note \a lastUsed time is not incremented but updated using setLastUsed().
    UsageStatistics& operator+=(const UsageStatistics& rhs) {
        m_TotalCount += rhs.m_TotalCount;
        m_LastWeekCount += rhs.m_LastWeekCount;
        m_LastTrimCount += rhs.m_LastTrimCount;
        m_HaveCalled += rhs.m_TotalCount; // '+=' mean '|=' here
        setLastUsed(rhs.m_LastUsed);
        return *this;
    }

private:
    unsigned m_TotalCount {0};    ///< cummulated usage in number of time
    unsigned m_TotalSeconds {0};  ///< cummulated usage in number of seconds
    unsigned m_LastWeekCount {0}; ///< XXX: not documented, not clear
    unsigned m_LastTrimCount {0}; ///< XXX: not documented, not clear
    time_t m_LastUsed {0};        ///< last usage time
    bool m_HaveCalled {false};    ///< has object been called? (used for call type object)
};
