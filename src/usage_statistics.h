/****************************************************************************
 *   Copyright (C) 2017 by Savoir-faire Linux                          *
 *   Author : Guillaume Roguez <guillaume.roguez@savoirfairelinux.com> *
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

/**
 * UsageStatistics is a container to store common statistics about
 * usage of arbitrary object.
 */
class UsageStatistics
{
public:
    unsigned totalCount {0}; //< object usage in number of time
    unsigned lastWeekCount {0}; //< object usage in number of weeks
    unsigned lastTrimCount {0}; //< object usage in number of quarters
    bool haveCalled {false}; //< has object been called? (used for call type object)
    time_t lastUsed {0}; //< time of last usage

public:
    /**
     * Use this method to update lastUsed time by a new time only if sooner.
     * \return true if the update has been effective.
     */
    bool setLastUsed(time_t time) {
        if (lastUsed < time) {
            lastUsed = time;
            return true;
        }
        return false;
    }

    /**
     * inplace incrementation of current values by values from another UsageStatistics instance.
     *
     * Note: lastUsed time is not incremented but updated using setLastUsed().
     */
    UsageStatistics& operator+=(const UsageStatistics& rhs) {
        totalCount += rhs.totalCount;
        lastWeekCount += rhs.lastWeekCount;
        lastTrimCount += rhs.lastTrimCount;
        haveCalled += rhs.totalCount; // '+=' mean '|=' here
        setLastUsed(rhs.lastUsed);
        return *this;
    }
};
