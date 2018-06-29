/****************************************************************************
 *   Copyright (C) 2015-2018 Savoir-faire Linux                               *
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
#include "file.h"

//Dring
#include <media_const.h>
#include "dbus/callmanager.h"

//Ring

class MediaFilePrivate
{
};

media::File::File(void* parent, const Media::Direction direction) : media::Media(parent, direction), d_ptr(new MediaFilePrivate())
{
   Q_ASSERT(parent);
}

media::Media::Type media::File::type()
{
   return media::Media::Type::FILE;
}

media::File::~File()
{
   delete d_ptr;
}
