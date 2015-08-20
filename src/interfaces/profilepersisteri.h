/****************************************************************************
 *   Copyright (C) 2013-2015 by Savoir-faire Linux                          *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com>      *
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
#ifndef PROFILEPERSISTERI_H
#define PROFILEPERSISTERI_H

#include <typedefs.h>

// Qt
class QDir;

// Ring
class Person;

namespace Interfaces {

class ProfilePersisterI {
public:
    virtual ~ProfilePersisterI() = default;

    virtual bool load       (               )       = 0;
    virtual bool save       (const Person* c)       = 0;
    virtual QDir profilesDir(               ) const = 0;
};

} // namespace Interfaces

#endif // PROFILEPERSISTERI_H
