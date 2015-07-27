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
#include "dbus/configurationmanager.h"

//Ring
#include <call.h>
#include <callmodel.h>
#include <account.h>
#include <media/textrecording.h>
#include <media/recordingmodel.h>
#include <phonedirectorymodel.h>
#include <private/call_p.h>
#include <private/textrecording_p.h>
#include <private/imconversationmanagerprivate.h>

/*
 * Instant message have 3 major modes, "past", "in call" and "offline"
 *
 * Offline messages are currently implemented over the DHT and may eventually
 * be implemented over account registration (publish--subscribe)
 *
 * In call messages are the fastest as they have a communication channels of their
 * own.
 *
 * Past messages are local copy of past communication stored somewhere.
 *
 * All 3 sources have to be combined dynamically into a single stream per person.
 *
 * So chunks can come from all 3 sources for the same stream, including part that
 * are shared in a conference with multiple peers.
 */

class MediaTextPrivate
{
public:
   MediaTextPrivate(Media::Text* parent);

   //Attributes
   Media::TextRecording* m_pRecording;
   bool m_HasChecked;

private:
   Media::Text* q_ptr;
};


IMConversationManagerPrivate::IMConversationManagerPrivate(QObject* parent) : QObject(parent)
{
   CallManagerInterface& callManager                   = DBus::CallManager::instance();
   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();

   connect(&configurationManager, &ConfigurationManagerInterface::incomingAccountMessage, this, &IMConversationManagerPrivate::newAccountMessage);
   connect(&callManager         , &CallManagerInterface::incomingMessage                , this, &IMConversationManagerPrivate::newMessage       );
}

IMConversationManagerPrivate* IMConversationManagerPrivate::instance()
{
   static IMConversationManagerPrivate* ins = new IMConversationManagerPrivate(nullptr);

   return ins;
}

///Called when a new message is incoming
void IMConversationManagerPrivate::newMessage(const QString& callId, const QString& from, const QMap<QString,QString>& message)
{
   Q_UNUSED(from)

   Call* call = CallModel::instance()->getCall(callId);

   Q_ASSERT(call);

   qDebug() << "Creating messaging model for call" << callId;
   Media::Text* media = call->firstMedia<Media::Text>(Media::Media::Direction::IN);

   if (!media) {
      media = call->d_ptr->mediaFactory<Media::Text>(Media::Media::Direction::IN);
   }

   media->recording()->setCall(call);
   media->recording()->d_ptr->insertNewMessage(message,call->peerContactMethod(),Media::Media::Direction::IN);
   emit media->messageReceived(message);
}

void IMConversationManagerPrivate::newAccountMessage(const QString& accountId, const QString& from, const QString& message)
{
   qDebug() << "GOT MESSAGE" << accountId << from << message;
}

MediaTextPrivate::MediaTextPrivate(Media::Text* parent) : q_ptr(parent),m_pRecording(nullptr),m_HasChecked(false)
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
   delete d_ptr;
}

Media::TextRecording* Media::Text::recording() const
{
   const bool wasChecked = d_ptr->m_HasChecked;
   d_ptr->m_HasChecked = true;

   if ((!wasChecked) && !d_ptr->m_pRecording) {
      Text* other = call()->firstMedia<Text>(direction() == Media::Direction::OUT ?
         Media::Direction::IN
      :  Media::Direction::OUT
      );

      if (other && other->recording())
         d_ptr->m_pRecording = other->recording();

   }

   if ((!wasChecked) && !d_ptr->m_pRecording) {
      d_ptr->m_pRecording = RecordingModel::instance()->createTextRecording(call()->peerContactMethod());
   }

   return d_ptr->m_pRecording;
}

/**
 * Send a message to the peer.
 *
 * @param message A messages encoded in various alternate payloads
 *
 * This send a single message. Just as e-mails, the message can be
 * encoded differently. The peer client will interpret the richest
 * payload it support and then fallback to lesser ones.
 * 
 * Suggested payloads include:
 *
 * "text/plain"    : The most common plain UTF-8 text
 * "text/enriched" : (RTF) The rich text format used by older applications (like WordPad and OS X TextEdit)
 * "text/html"     : The format used by web browsers
 */
void Media::Text::send(const QMap<QString,QString>& message)
{
   CallManagerInterface& callManager = DBus::CallManager::instance();
   Q_NOREPLY callManager.sendTextMessage(call()->dringId(), message);

   //Make sure the recording exist
   recording();

   d_ptr->m_pRecording->setCall(call());
   d_ptr->m_pRecording->d_ptr->insertNewMessage(message,call()->account()->contactMethod(),Media::Direction::OUT);

   emit messageSent(message);
}

#include <text.moc>
