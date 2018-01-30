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
#include "audio.h"

//Dring
#include <media_const.h>
#include "dbus/callmanager.h"

//Ring
#include <call.h>

class MediaAudioPrivate
{
};

Media::Audio::Audio(Call* parent, const Media::Direction direction) : Media::Media(parent, direction), d_ptr(new MediaAudioPrivate())
{
   Q_ASSERT(parent);
}

Media::Media::Type Media::Audio::type()
{
   return Media::Media::Type::AUDIO;
}

bool Media::Audio::mute()
{
   CallManagerInterface& callManager = CallManager::instance();
   return callManager.muteLocalMedia(call()->dringId(),DRing::Media::Details::MEDIA_TYPE_AUDIO,true);
}

bool Media::Audio::unmute()
{
   CallManagerInterface& callManager = CallManager::instance();
   return callManager.muteLocalMedia(call()->dringId(),DRing::Media::Details::MEDIA_TYPE_AUDIO,false);
}

Media::Audio::~Audio()
{
   delete d_ptr;
}