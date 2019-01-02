/****************************************************************************
 *    Copyright (C) 2017-2019 Savoir-faire Linux Inc.                               *
 *   Author : Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>      *
 *   Author : Stepan Salenikovich <stepan.salenikovich@savoirfairelinux.com>*
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

#include "usage_statistics.h"

//Qt
#include <QDebug>

//Accessors

unsigned
UsageStatistics::totalCount() const {
    return m_TotalSeconds;
}

unsigned
UsageStatistics::totalSeconds() const {
    return m_TotalSeconds;
}

unsigned
UsageStatistics::lastWeekCount() const {
    return m_LastWeekCount;
}

unsigned
UsageStatistics::lastTrimCount() const {
    return m_LastTrimCount;
}

time_t
UsageStatistics::lastUsed() const {
    return m_LastUsed;
}

bool
UsageStatistics::haveCalled() const {
    return m_HaveCalled;
}

//Mutators

void
UsageStatistics::setHaveCalled() {
    m_HaveCalled = true;
}

/// \brief Update usage using a time range.
///
/// All values are updated using given <tt>[start, stop]</tt> time range.
/// \a start and \a stop are given in seconds.
///
/// \param start starting time of usage
/// \param stop ending time of usage, must be greater than \a start
void
UsageStatistics::update(const time_t& start, const time_t& stop) {
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
bool
UsageStatistics::setLastUsed(time_t new_time) {
    if (new_time > m_LastUsed) {
        m_LastUsed = new_time;
        return true;
    }
    return false;
}

UsageStatistics& UsageStatistics::operator+=(const UsageStatistics& rhs) {
    m_TotalCount += rhs.m_TotalCount;
    m_LastWeekCount += rhs.m_LastWeekCount;
    m_LastTrimCount += rhs.m_LastTrimCount;
    m_HaveCalled += rhs.m_TotalCount; // '+=' mean '|=' here
    setLastUsed(rhs.m_LastUsed);
    return *this;
}
