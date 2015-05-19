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

QHash<QByteArray, Serializable::Peers*> SerializableEntityManager::m_hPeers;


Serializable::Peers* SerializableEntityManager::peer(ContactMethod* cm)
{
   const QByteArray sha1 = cm->sha1();
   Serializable::Peers* p = m_hPeers[sha1];

   if (!p) {
      p = new Serializable::Peers();
      m_hPeers[sha1] = p;
   }

   return p;
}

Serializable::Peers* SerializableEntityManager::peers(QList<ContactMethod*> cms)
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

Media::TextRecordingPrivate::TextRecordingPrivate(TextRecording* r) : q_ptr(r),m_pImModel(nullptr)
{

}

Media::TextRecording::TextRecording(const Recording::Type type) : Recording(type), d_ptr(new TextRecordingPrivate(this))
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
      
   }

   return d_ptr->m_pImModel;
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
      Message message;
      message.read(o);
      messages.append(message);
   }
}

void Serializable::Group::write(QJsonObject &json) const
{
   json["id"            ] = id           ;
   json["nextGroupSha1" ] = nextGroupSha1;
   json["nextGroupId"   ] = nextGroupId  ;

   QJsonArray a;
   for (const Message& m : messages) {
      QJsonObject o;
      m.write(o);
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
      Group group;
      group.read(o);
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
   for (const Group& g : groups) {
      QJsonObject o;
      g.write(o);
      a.append(o);
   }
   json["groups"] = a;
}
