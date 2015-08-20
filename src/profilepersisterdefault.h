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
#ifndef PROFILEPERSISTERDEFAULT_H
#define PROFILEPERSISTERDEFAULT_H

#include <typedefs.h>

#include "interfaces/profilepersisteri.h"

// Qt
// #include <QtCore/QDir>
class QDir;

// Ring
class Person;

namespace Interfaces {

class LIB_EXPORT ProfilePersisterDefault : public ProfilePersisterI {
public:
    // FIXME: the load and save methods should be taken from the implementation in profilemodel.cpp
    bool load       (               )       override;
    bool save       (const Person* c)       override;
    QDir profilesDir(               ) const override;
};

} // namespace Interfaces

#endif // PROFILEPERSISTERDEFAULT_H
