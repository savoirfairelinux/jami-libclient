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
#ifndef PROFILEPERSISTERDELEGATE_H
#define PROFILEPERSISTERDELEGATE_H

#include <typedefs.h>

// Qt
#include <QtCore/QDir>

// Ring
class Person;

namespace Delegates {

class LIB_EXPORT ProfilePersisterDelegate {
public:
    virtual ~ProfilePersisterDelegate() {}

    // FIXME: the load and save methods should be taken from the implementation in profilemodel.cpp
    virtual bool load() = 0;
    virtual bool save(const Person* c) = 0;
    virtual QDir getProfilesDir();
};

} // namespace Delegates

#endif // PROFILEPERSISTERVISITOR_H
