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
#include "textrecording.h"

//Qt
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonValue>
#include <QtCore/QCryptographicHash>

//Ring
#include <callmodel.h>
#include <contactmethod.h>
#include <instantmessagingmodel.h>
#include <private/textrecording_p.h>
#include <private/instantmessagingmodel_p.h>

//Std
#include <ctime>

QHash<QByteArray, Serializable::Peers*> SerializableEntityManager::m_hPeers;


Serializable::Peers* SerializableEntityManager::peer(const ContactMethod* cm)
{
   const QByteArray sha1 = cm->sha1();
   Serializable::Peers* p = m_hPeers[sha1];

   if (!p) {
      p = new Serializable::Peers();
      m_hPeers[sha1] = p;
   }

   return p;
}

Serializable::Peers* SerializableEntityManager::peers(QList<const ContactMethod*> cms)
{
   QByteArray ps;

   for(const ContactMethod* cm : cms) {
      ps += cm->sha1();
   }

   QCryptographicHash hash(QCryptographicHash::Sha1);
   hash.addData(ps);

   //Create a reproducible key for this file
   const QByteArray sha1 = hash.result().toHex();

   Serializable::Peers* p = m_hPeers[sha1];

   if (!p) {
      p = new Serializable::Peers();
      m_hPeers[sha1] = p;
   }

   return p;
}

Media::TextRecordingPrivate::TextRecordingPrivate(TextRecording* r) : q_ptr(r),m_pImModel(nullptr),m_pCurrentGroup(nullptr)
{

}

Media::TextRecording::TextRecording() : Recording(Recording::Type::TEXT), d_ptr(new TextRecordingPrivate(this))
{
}

Media::TextRecording::~TextRecording()
{
   delete d_ptr;
}

///Get the instant messaging model associated with this recording
InstantMessagingModel* Media::TextRecording::instantMessagingModel() const
{
   if (!d_ptr->m_pImModel) {
      d_ptr->m_pImModel = new InstantMessagingModel(const_cast<TextRecording*>(this));
   }

   return d_ptr->m_pImModel;
}

void Media::TextRecordingPrivate::insertNewMessage(const QString& message, const ContactMethod* cm, Media::Media::Direction direction)
{

   //Only create it if none was found on the disk
   if (!m_pCurrentGroup) {
      m_pCurrentGroup = new Serializable::Group();
      Serializable::Peers* p = SerializableEntityManager::peer(cm);
      p->groups << m_pCurrentGroup;
   }

   //Create the message
   time_t currentTime;
   ::time(&currentTime);
   Serializable::Message* m = new Serializable::Message();

   m->timestamp = currentTime                      ;
   m->payload   = message                          ;
   m->mimeType  = "text/plain"                     ;
   m->direction = direction                        ;
   m->type      = Serializable::Message::Type::CHAT;
   m_pCurrentGroup->messages << m;

   //Make sure the model exist
   q_ptr->instantMessagingModel();

   //Update the reconstructed conversation
   ::TextMessageNode* n  = new ::TextMessageNode()       ;
   n->m_pMessage         = m                             ;
   n->m_pContactMethod   = const_cast<ContactMethod*>(cm);
   m_pImModel->d_ptr->addRowBegin();
   m_lNodes << n;
   m_pImModel->d_ptr->addRowEnd();
}

void Media::TextRecordingPrivate::save()
{
   for (Serializable::Peers* p : m_lAssociatedPeers) {
      if (p->hasChanged) {
         p->hasChanged = false;
      }
   }
}

void Serializable::Message::read (const QJsonObject &json)
{
   timestamp = json["timestamp"].toInt                 (                           );
   payload   = json["payload"  ].toString              (                           );
   mimeType  = json["mimeType" ].toString              (                           );
   author    = json["author"   ].toString              (                           );
   direction = static_cast<Media::Media::Direction>    ( json["direction"].toInt() );
   type      = static_cast<Serializable::Message::Type>( json["type"     ].toInt() );
}

void Serializable::Message::write(QJsonObject &json) const
{
   json["timestamp"] = static_cast<int>(timestamp);
   json["payload"  ] = payload                    ;
   json["mimeType" ] = mimeType                   ;
   json["author"   ] = author                     ;
   json["direction"] = static_cast<int>(direction);
   json["type"     ] = static_cast<int>(type)     ;
}

void Serializable::Group::read (const QJsonObject &json)
{
   id            = json["id"           ].toInt   ();
   nextGroupSha1 = json["nextGroupSha1"].toString();
   nextGroupId   = json["nextGroupId"  ].toInt   ();

   QJsonArray a = json["messages"].toArray();
   for (int i = 0; i < a.size(); ++i) {
      QJsonObject o = a[i].toObject();
      Message* message = new Message();
      message->read(o);
      messages.append(message);
   }
}

void Serializable::Group::write(QJsonObject &json) const
{
   json["id"            ] = id           ;
   json["nextGroupSha1" ] = nextGroupSha1;
   json["nextGroupId"   ] = nextGroupId  ;

   QJsonArray a;
   for (const Message* m : messages) {
      QJsonObject o;
      m->write(o);
      a.append(o);
   }
   json["messages"] = a;
}

void Serializable::Peers::read (const QJsonObject &json)
{

   QJsonArray as = json["sha1s"].toArray();
   for (int i = 0; i < as.size(); ++i) {
      sha1s.append(as[i].toString());
   }

   QJsonArray a = json["groups"].toArray();
   for (int i = 0; i < a.size(); ++i) {
      QJsonObject o = a[i].toObject();
      Group* group = new Group();
      group->read(o);
      groups.append(group);
   }
}

void Serializable::Peers::write(QJsonObject &json) const
{
   QJsonArray a2;
   for (const QString& sha1 : sha1s) {
      a2.append(sha1);
   }
   json["sha1s"] = a2;

   QJsonArray a;
   for (const Group* g : groups) {
      QJsonObject o;
      g->write(o);
      a.append(o);
   }
   json["groups"] = a;
}
