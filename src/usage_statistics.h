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

class UsageStatistics {
public:
    unsigned totalCount {0};
    unsigned lastWeekCount {0};
    unsigned lastTrimCount {0};
    bool haveCalled {false};
    time_t lastUsed {0};

public:
    UsageStatistics& operator+=(const UsageStatistics& rhs) {
        totalCount += rhs.totalCount;
        lastWeekCount += rhs.lastWeekCount;
        lastTrimCount += rhs.lastTrimCount;
        haveCalled += rhs.totalCount; // '+=' mean '|=' here
        if (lastUsed < rhs.lastUsed)
            lastUsed = rhs.lastUsed;
        return *this;
    }
};
