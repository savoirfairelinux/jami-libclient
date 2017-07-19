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
#include "video.h"

//Dring
#include <media_const.h>
#include <video/sourcemodel.h>
#include "dbus/callmanager.h"

//Ring
#include <call.h>

class MediaVideoPrivate
{
public:
    Video::SourceModel *m_pSourceModel = nullptr;
};

Media::Video::Video(Call* parent, const Media::Direction direction) : Media::Media(parent, direction), d_ptr(new MediaVideoPrivate())
{
   Q_ASSERT(parent);
}

Media::Media::Type Media::Video::type()
{
   return Media::Media::Type::VIDEO;
}

bool Media::Video::mute()
{
   CallManagerInterface& callManager = CallManager::instance();
   return callManager.muteLocalMedia(call()->dringId(),DRing::Media::Details::MEDIA_TYPE_VIDEO,true);
}

bool Media::Video::unmute()
{
   CallManagerInterface& callManager = CallManager::instance();
   return callManager.muteLocalMedia(call()->dringId(),DRing::Media::Details::MEDIA_TYPE_VIDEO,false);
}

Video::SourceModel* Media::Video::sourceModel() const
{
    if (!d_ptr->m_pSourceModel) {
        d_ptr->m_pSourceModel = new ::Video::SourceModel();
    }

    return d_ptr->m_pSourceModel;
}

Media::Video::~Video()
{
   delete d_ptr;
}
