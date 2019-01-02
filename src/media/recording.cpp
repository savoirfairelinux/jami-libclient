/****************************************************************************
 *    Copyright (C) 2015-2019 Savoir-faire Linux Inc.                               *
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

namespace media {

class RecordingPrivate {
public:
   RecordingPrivate(Recording* r);

   //Attributes
   Recording::Type        m_Type    ;
   Call*                  m_pCall   ;

private:
   Recording* q_ptr;
};

RecordingPrivate::RecordingPrivate(Recording* r) : q_ptr(r),m_pCall(nullptr)
{

}

Recording::Recording(const Recording::Type type) : ItemBase(nullptr), d_ptr(new RecordingPrivate(this))
{
   d_ptr->m_Type = type;
}

Recording::~Recording()
{
   delete d_ptr;
}

} //Media::

///Return this Recording type
media::Recording::Type media::Recording::type() const
{
   return d_ptr->m_Type;
}

Call* media::Recording::call() const
{
   return d_ptr->m_pCall;
}

void media::Recording::setCall(Call* call)
{
   d_ptr->m_pCall = call;
}

#include <recording.moc>
