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
#ifndef MEDIA_AUDIO_H
#define MEDIA_AUDIO_H

#include <media/media.h>
#include <typedefs.h>

class MediaAudioPrivate;
class Call;

namespace Media {

class LIB_EXPORT Audio : Media::Media
{
public:

   virtual Media::Type type() override;
   virtual bool mute() override;
   virtual bool unmute() override;

private:
   Audio(Call* parent);
   virtual ~Audio();

   MediaAudioPrivate* d_ptr;
};

}

#endif
