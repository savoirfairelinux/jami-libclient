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
#include "text.h"

//Dring
#include <media_const.h>
#include "dbus/callmanager.h"

//Ring
#include <call.h>
#include <imconversationmanager.h>
#include <instantmessagingmodel.h>
#include <private/call_p.h>
#include <private/instantmessagingmodel_p.h>

class MediaTextPrivate
{
public:
   MediaTextPrivate(Media::Text* parent);

   //Attributes
   InstantMessagingModel* m_pImModel;

private:
   Media::Text* q_ptr;
};

MediaTextPrivate::MediaTextPrivate(Media::Text* parent) : q_ptr(parent),m_pImModel(nullptr)
{

}

Media::Text::Text(Call* parent, const Media::Direction direction) : Media::Media(parent, direction), d_ptr(new MediaTextPrivate(this))
{
   Q_ASSERT(parent);
}

Media::Media::Type Media::Text::type()
{
   return Media::Media::Type::TEXT;
}

Media::Text::~Text()
{

}

///Send a text message
void Media::Text::send(const QString& message)
{
   CallManagerInterface& callManager = DBus::CallManager::instance();
   Q_NOREPLY callManager.sendTextMessage(call()->dringId(),message);
   if (!d_ptr->m_pImModel) {
      d_ptr->m_pImModel = IMConversationManager::instance()->getModel(call());
   }
   d_ptr->m_pImModel->d_ptr->addOutgoingMessage(message);

   emit messageSent(message);
}
