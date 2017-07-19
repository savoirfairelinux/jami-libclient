/****************************************************************************
 *   Copyright (C) 2015-2017 Savoir-faire Linux                               *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
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
#include "ringtone.h"

//Qt
#include <QtCore/QUrl>
#include <QtCore/QDir>

class RingtonePrivate
{
public:
   RingtonePrivate();
   QString m_Path;
   QString m_Name;
};

RingtonePrivate::RingtonePrivate()
{

}

Ringtone::Ringtone(QObject* parent) : ItemBase(parent), d_ptr(new RingtonePrivate())
{

}

Ringtone::~Ringtone()
{
   delete d_ptr;
}

QString Ringtone::path() const
{
   return d_ptr->m_Path;
}

QString Ringtone::name() const
{
   return d_ptr->m_Name;
}

void Ringtone::setPath(const QString& path)
{
   d_ptr->m_Path = QDir::toNativeSeparators(path);
}

void Ringtone::setName(const QString& name)
{
   d_ptr->m_Name = name;
}
