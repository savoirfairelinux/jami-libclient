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
#include "profilepersisterdefault.h"

// Qt
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>

namespace Interfaces {

bool ProfilePersisterDefault::load()
{
    //FIXME: move load method from profilemodel here
    return false;
}

bool ProfilePersisterDefault::save(const Person* c)
{
    Q_UNUSED(c)
    //FIXME: move save method from profilemodel here
    return false;
}

QDir ProfilePersisterDefault::profilesDir() const
{
    static QDir d = (QStandardPaths::writableLocation(QStandardPaths::DataLocation)) + "/profiles/";
    return d;
}

} // namespace Interfaces
