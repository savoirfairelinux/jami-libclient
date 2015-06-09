/****************************************************************************
 *   Copyright (C) 2015 by Savoir-Faire Linux                               *
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

class RingtonePrivate
{
public:
   RingtonePrivate();
   QString m_Path;
   QString m_Name;
   bool    m_IsPlaying;
};

RingtonePrivate::RingtonePrivate() : m_IsPlaying(false)
{

}

Ringtone::Ringtone(QObject* parent) : ItemBase<QObject>(parent), d_ptr(new RingtonePrivate())
{

}

Ringtone::~Ringtone()
{
   delete d_ptr;
}

QUrl Ringtone::path() const
{
   return d_ptr->m_Path;
}

QString Ringtone::name() const
{
   return d_ptr->m_Name;
}


void Ringtone::setPath(const QUrl& url)
{
   d_ptr->m_Path = url.path();
}

void Ringtone::setName(const QString& name)
{
   d_ptr->m_Name = name;
}

bool Ringtone::isPlaying() const
{
   return d_ptr->m_IsPlaying;
}

void Ringtone::setIsPlaying(const bool value)
{
   d_ptr->m_IsPlaying = value;
}
