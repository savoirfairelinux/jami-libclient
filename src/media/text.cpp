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
#include <phonedirectorymodel.h>
#include <instantmessagingmodel.h>
#include <private/call_p.h>
#include <private/instantmessagingmodel_p.h>
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
   InstantMessagingModel* m_pImModel;

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
void IMConversationManagerPrivate::newMessage(const QString& callId, const QString& from, const QString& message)
{
   if (!m_lModels[callId] && CallModel::instance()) {
      Call* call = CallModel::instance()->getCall(callId);
      if (call) {
         qDebug() << "Creating messaging model for call" << callId;
         m_lModels[callId] = new InstantMessagingModel(call);
         m_lModels[callId]->d_ptr->addIncommingMessage(from,message);
      }
   }
   else if (m_lModels[callId]) {
      m_lModels[callId]->d_ptr->addIncommingMessage(from,message);
   }
}

void IMConversationManagerPrivate::newAccountMessage(const QString& accountId, const QString& from, const QString& message)
{
   qDebug() << "GOT MESSAGE" << accountId << from << message;
}

InstantMessagingModel* IMConversationManagerPrivate::createModel(Media::Text* t, Account* a, const QString& peerNumber)
{
   InstantMessagingModel* imm = new InstantMessagingModel(t->call(), this);

   if (t) {
      if (t->call()->hasRemote()) {
         m_lModels[t->call()->dringId()] = imm;
      }
      else {
         m_lNewCallModels[t->call()] = imm;
      }
      m_lOutOfCallsModels[t->call()->peerContactMethod()] = imm;
   }
   else if (a && !peerNumber.isEmpty()) {
      ContactMethod* n = PhoneDirectoryModel::instance()->getNumber(peerNumber, a);
      m_lOutOfCallsModels[n] = imm;
   }
   else {
      delete imm;

      return nullptr;
   }

   return imm;
}

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

InstantMessagingModel* Media::Text::instantMessagingModel() const
{
   if (!d_ptr->m_pImModel) {
      d_ptr->m_pImModel = IMConversationManagerPrivate::instance()->createModel(const_cast<Text*>(this));
   }
   return d_ptr->m_pImModel;
}

///Send a text message
void Media::Text::send(const QString& message)
{
   CallManagerInterface& callManager = DBus::CallManager::instance();
   Q_NOREPLY callManager.sendTextMessage(call()->dringId(),message);

   //Make sure it exist
   instantMessagingModel();

   d_ptr->m_pImModel->d_ptr->addOutgoingMessage(message);

   emit messageSent(message);
}

#include <text.moc>
