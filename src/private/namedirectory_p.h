/****************************************************************************
 *    Copyright (C) 2016-2019 Savoir-faire Linux Inc.                               *
 *   Author : Alexandre Viau <alexandre.viau@savoirfairelinux.com>          *
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

#include "namedirectory.h"

typedef void (NameDirectoryPrivate::*NameDirectoryPrivateFct)();

class NameDirectoryPrivate: public QObject
{
    Q_OBJECT

private:
    NameDirectory* q_ptr;

public:
    NameDirectoryPrivate(NameDirectory*);

public Q_SLOTS:
    void slotNameRegistrationEnded(const std::string& accountId, const int32_t& status, const std::string& name);
    void slotRegisteredNameFound(const std::string& accountId, const int32_t& status, const std::string& address, const std::string& name);

};
