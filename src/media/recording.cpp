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
#include "recording.h"

#include <QtCore/QUrl>

namespace Media {

class RecordingPrivate {
public:
   RecordingPrivate(Recording* r);

   //Attributes
   QUrl m_Path;
   Recording::Type m_Type;

   Recording* q_ptr;
};

RecordingPrivate::RecordingPrivate(Recording* r) : q_ptr(r)
{

}

Recording::Recording(const Recording::Type type) : ItemBase<QObject>(nullptr), d_ptr(new RecordingPrivate(this))
{
   d_ptr->m_Type = type;
}

Recording::~Recording()
{
   delete d_ptr;
}

} //Media::

Media::Recording::Type Media::Recording::type() const
{
   return d_ptr->m_Type;
}

QUrl Media::Recording::path() const
{
   return d_ptr->m_Path;
}

void Media::Recording::setPath(const QUrl& path)
{
   d_ptr->m_Path = path;
}

void Media::Recording::play()
{
   //TODO
}

void Media::Recording::pause()
{
   //TODO
}

void Media::Recording::seek(Recording::Position pos)
{
   Q_UNUSED(pos)
   //TODO
}

void Media::Recording::reset()
{
   //TODO
}
