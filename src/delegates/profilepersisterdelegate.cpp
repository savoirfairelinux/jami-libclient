/****************************************************************************
 *   Copyright (C) 2013-2014 by Savoir-Faire Linux                           *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com> *
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

#include "profilepersisterdelegate.h"
#include <QtCore/QSize>
#include <QtCore/QStandardPaths>

ProfilePersisterDelegate* ProfilePersisterDelegate::m_spInstance = nullptr;

void ProfilePersisterDelegate::setInstance(ProfilePersisterDelegate* i)
{
   m_spInstance = i;
}

ProfilePersisterDelegate* ProfilePersisterDelegate::instance()
{
   return m_spInstance;
}

bool ProfilePersisterDelegate::load()
{
   return false;
}

bool ProfilePersisterDelegate::save(const Person* c)
{
   Q_UNUSED(c)
   return false;
}

QDir ProfilePersisterDelegate::getProfilesDir()
{
   static QDir d = (QStandardPaths::writableLocation(QStandardPaths::DataLocation)) + "/profiles/";
   return d;
}
