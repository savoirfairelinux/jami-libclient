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
#include <QtCore/QJsonDocument>
#include <QtCore/QDateTime>
#include <QtCore/QCryptographicHash>

//Ring
#include <callmodel.h>
#include <contactmethod.h>
#include <account.h>
#include <phonedirectorymodel.h>
#include <accountmodel.h>
#include <personmodel.h>
#include <private/textrecording_p.h>
#include "delegates/delegatemanager.h"
#include "delegates/pixmapmanipulationdelegate.h"

//Std
#include <ctime>

QHash<QByteArray, Serializable::Peers*> SerializableEntityManager::m_hPeers;

void addPeer(Serializable::Peers* p,  const ContactMethod* cm);
void addPeer(Serializable::Peers* p,  const ContactMethod* cm)
{
   Serializable::Peer* peer = new Serializable::Peer();
   peer->sha1      = cm->sha1();
   peer->uri       = cm->uri();
   peer->accountId = cm->account() ? cm->account()->id () : QString();
   peer->personUID = cm->contact() ? cm->contact()->uid() : QString();
   p->peers << peer;
}

Serializable::Peers* SerializableEntityManager::peer(const ContactMethod* cm)
{
   const QByteArray sha1 = cm->sha1();
   Serializable::Peers* p = m_hPeers[sha1];

   if (!p) {
      p = new Serializable::Peers();
      p->sha1s << sha1;

      addPeer(p,cm);

      m_hPeers[sha1] = p;
   }

   return p;
}

QByteArray mashSha1s(QList<QString> sha1s);
QByteArray mashSha1s(QList<QString> sha1s)
{
   QCryptographicHash hash(QCryptographicHash::Sha1);

   QByteArray ps;

   for (const QString& sha1 : sha1s) {
      ps += sha1.toLatin1();
   }

   hash.addData(ps);

   //Create a reproducible key for this file
   return hash.result().toHex();
}

Serializable::Peers* SerializableEntityManager::peers(QList<const ContactMethod*> cms)
{
   QList<QString> sha1s;

   for(const ContactMethod* cm : cms) {
      const QByteArray sha1 = cm->sha1();
      sha1s << sha1;
   }

   const QByteArray sha1 = ::mashSha1s(sha1s);

   Serializable::Peers* p = m_hPeers[sha1];

   if (!p) {
      p = new Serializable::Peers();
      p->sha1s = sha1s;
      m_hPeers[sha1] = p;
   }

   return p;
}

Serializable::Peers* SerializableEntityManager::fromSha1(const QByteArray& sha1)
{
   return m_hPeers[sha1];
}

Serializable::Peers* SerializableEntityManager::fromJson(const QJsonObject& json, const ContactMethod* cm)
{
   //Check if the object is already loaded
   QStringList sha1List;
   QJsonArray as = json["sha1s"].toArray();
   for (int i = 0; i < as.size(); ++i) {
      sha1List.append(as[i].toString());
   }

   if (sha1List.isEmpty())
      return nullptr;

   QByteArray sha1 = sha1List[0].toLatin1();

   if (sha1List.size() > 1) {
      sha1 = mashSha1s(sha1List);
   }

   if (m_hPeers[sha1])
      return m_hPeers[sha1];

   //Load from json
   Serializable::Peers* p = new Serializable::Peers();
   p->read(json);
   m_hPeers[sha1] = p;

   //TODO Remove in 2016
   //Some older versions of the file don't store necessary values, fix that
   if (cm && p->peers.isEmpty())
      addPeer(p,cm);

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
QAbstractListModel* Media::TextRecording::instantMessagingModel() const
{
   if (!d_ptr->m_pImModel) {
      d_ptr->m_pImModel = new InstantMessagingModel(const_cast<TextRecording*>(this));
   }

   return d_ptr->m_pImModel;
}

QHash<QByteArray,QByteArray> Media::TextRecordingPrivate::toJsons() const
{
   QHash<QByteArray,QByteArray> ret;
   for (Serializable::Peers* p : m_lAssociatedPeers) {
//       if (p->hasChanged) {
         p->hasChanged = false;

         QJsonObject output;
         p->write(output);

         QJsonDocument doc(output);
         ret[p->sha1s[0].toLatin1()] = doc.toJson();
//       }
   }

   return ret;
}

Media::TextRecording* Media::TextRecording::fromJson(const QList<QJsonObject>& items, const ContactMethod* cm)
{
   TextRecording* t = new TextRecording();

   //Load the history data
   for (const QJsonObject& obj : items) {
      Serializable::Peers* p = SerializableEntityManager::fromJson(obj,cm);
      t->d_ptr->m_lAssociatedPeers << p;
   }

   //Create the model
   t->instantMessagingModel();

   //Reconstruct the conversation
   //TODO do it right, right now it flatten the graph
   for (const Serializable::Peers* p : t->d_ptr->m_lAssociatedPeers) {
      for (const Serializable::Group* g : p->groups) {
         for (Serializable::Message* m : g->messages) {
            ::TextMessageNode* n  = new ::TextMessageNode();
            n->m_pMessage         = m                      ;
            if (!n->m_pMessage->contactMethod) {
               n->m_pMessage->contactMethod = const_cast<ContactMethod*>(cm); //TODO remove in 2016
               n->m_pMessage->authorSha1 = cm->sha1();

               if (p->peers.isEmpty())
                  addPeer(const_cast<Serializable::Peers*>(p), cm);
            }
            n->m_pContactMethod   = m->contactMethod;
            t->d_ptr->m_pImModel->addRowBegin();
            t->d_ptr->m_lNodes << n;
            t->d_ptr->m_pImModel->addRowEnd();
         }
      }
   }

   return t;
}

void Media::TextRecordingPrivate::insertNewMessage(const QMap<QString,QString>& message, const ContactMethod* cm, Media::Media::Direction direction)
{

   //Only create it if none was found on the disk
   if (!m_pCurrentGroup) {
      m_pCurrentGroup = new Serializable::Group();
      Q_ASSERT(q_ptr->call());
      Serializable::Peers* p = SerializableEntityManager::peer(q_ptr->call()->peerContactMethod());

      if (m_lAssociatedPeers.indexOf(p) == -1) {
         m_lAssociatedPeers << p;
      }

      p->groups << m_pCurrentGroup;
   }

   //Create the message
   time_t currentTime;
   ::time(&currentTime);
   Serializable::Message* m = new Serializable::Message();

   m->timestamp = currentTime                      ;
   m->direction = direction                        ;
   m->type      = Serializable::Message::Type::CHAT;
   m->authorSha1= cm->sha1()                       ;

   QMapIterator<QString, QString> iter(message);
   while (iter.hasNext()) {
      iter.next();
      if (iter.value() != "application/resource-lists+xml") { //This one is useless
         Serializable::Payload* p = new Serializable::Payload();
         p->mimeType = iter.key  ();
         p->payload  = iter.value();
         m->payloads << p;

         if (p->mimeType == "text/plain")
            m->m_PlainText = p->payload;
         else if (p->mimeType == "text/html")
            m->m_HTML = p->payload;
      }
   }
   m_pCurrentGroup->messages << m;

   //Make sure the model exist
   q_ptr->instantMessagingModel();

   //Update the reconstructed conversation
   ::TextMessageNode* n  = new ::TextMessageNode()       ;
   n->m_pMessage         = m                             ;
   n->m_pContactMethod   = const_cast<ContactMethod*>(cm);
   m_pImModel->addRowBegin();
   m_lNodes << n;
   m_pImModel->addRowEnd();

   //Save the conversation
   q_ptr->save();
}

void Serializable::Payload::read(const QJsonObject &json)
{
   payload  = json["payload" ].toString();
   mimeType = json["mimeType"].toString();
}

void Serializable::Payload::write(QJsonObject& json) const
{
   json["payload" ] = payload ;
   json["mimeType"] = mimeType;
}

void Serializable::Message::read (const QJsonObject &json)
{
   timestamp  = json["timestamp" ].toInt                (                           );
   authorSha1 = json["authorSha1"].toString             (                           );
   isRead     = json["isRead"    ].toBool               (                           );
   direction  = static_cast<Media::Media::Direction>    ( json["direction"].toInt() );
   type       = static_cast<Serializable::Message::Type>( json["type"     ].toInt() );

   QJsonArray a = json["payloads"].toArray();
   for (int i = 0; i < a.size(); ++i) {
      QJsonObject o = a[i].toObject();
      Payload* p = new Payload();
      p->read(o);
      payloads << p;

      if (p->mimeType == "text/plain")
         m_PlainText = p->payload;
      else if (p->mimeType == "text/html")
         m_HTML = p->payload;
   }

   //Load older conversation from a time when only 1 mime/payload pair was supported
   if (!json["payload"   ].toString().isEmpty()) {
      Payload* p  = new Payload();
      p->payload  = json["payload"  ].toString();
      p->mimeType = json["mimeType" ].toString();
      payloads << p;
      m_PlainText = p->payload;
   }
}

void Serializable::Message::write(QJsonObject &json) const
{
   json["timestamp"  ] = static_cast<int>(timestamp);
   json["authorSha1" ] = authorSha1                 ;
   json["direction"  ] = static_cast<int>(direction);
   json["type"       ] = static_cast<int>(type)     ;
   json["isRead"     ] = isRead                     ;

   QJsonArray a;
   foreach (const Payload* p, payloads) {
      QJsonObject o;
      p->write(o);
      a.append(o);
   }
   json["payloads"] = a;
}

void Serializable::Group::read (const QJsonObject &json, const QHash<QString,ContactMethod*> sha1s)
{
   id            = json["id"           ].toInt   ();
   nextGroupSha1 = json["nextGroupSha1"].toString();
   nextGroupId   = json["nextGroupId"  ].toInt   ();

   QJsonArray a = json["messages"].toArray();
   for (int i = 0; i < a.size(); ++i) {
      QJsonObject o = a[i].toObject();
      Message* message = new Message();
      message->contactMethod = sha1s[message->authorSha1];
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

void Serializable::Peer::read (const QJsonObject &json)
{
   accountId = json["accountId"].toString();
   uri       = json["uri"      ].toString();
   personUID = json["personUID"].toString();
   sha1      = json["sha1"     ].toString();

   Account* a     = AccountModel::instance()->getById(accountId.toLatin1());
   Person* person = personUID.isEmpty() ?
      nullptr : PersonModel::instance()->getPersonByUid(personUID.toLatin1());

   m_pContactMethod = PhoneDirectoryModel::instance()->getNumber(uri,person,a);
}

void Serializable::Peer::write(QJsonObject &json) const
{
   json["accountId"] = accountId ;
   json["uri"      ] = uri       ;
   json["personUID"] = personUID ;
   json["sha1"     ] = sha1      ;
}

void Serializable::Peers::read (const QJsonObject &json)
{

   QJsonArray as = json["sha1s"].toArray();
   for (int i = 0; i < as.size(); ++i) {
      sha1s.append(as[i].toString());
   }

   QJsonArray a2 = json["peers"].toArray();
   for (int i = 0; i < a2.size(); ++i) {
      QJsonObject o = a2[i].toObject();
      Peer* peer = new Peer();
      m_hSha1[peer->sha1] = peer->m_pContactMethod;
      peer->read(o);
      peers.append(peer);
   }

   QJsonArray a = json["groups"].toArray();
   for (int i = 0; i < a.size(); ++i) {
      QJsonObject o = a[i].toObject();
      Group* group = new Group();
      group->read(o,m_hSha1);
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

   QJsonArray a3;
   for (const Peer* p : peers) {
      QJsonObject o;
      p->write(o);
      a3.append(o);
   }
   json["peers"] = a3;
}


///Constructor
InstantMessagingModel::InstantMessagingModel(Media::TextRecording* recording) : QAbstractListModel(recording),m_pRecording(recording)
{
}

InstantMessagingModel::~InstantMessagingModel()
{
//    delete d_ptr;
}

QHash<int,QByteArray> InstantMessagingModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
      roles.insert((int)Media::TextRecording::Role::Direction           , "direction"           );
      roles.insert((int)Media::TextRecording::Role::AuthorDisplayname   , "authorDisplayname"   );
      roles.insert((int)Media::TextRecording::Role::AuthorUri           , "authorUri"           );
      roles.insert((int)Media::TextRecording::Role::AuthorPresenceStatus, "authorPresenceStatus");
      roles.insert((int)Media::TextRecording::Role::Timestamp           , "timestamp"           );
      roles.insert((int)Media::TextRecording::Role::FormattedDate       , "formattedDate"       );
      roles.insert((int)Media::TextRecording::Role::IsStatus            , "isStatus"            );
   }
   return roles;
}

///Get data from the model
QVariant InstantMessagingModel::data( const QModelIndex& idx, int role) const
{
   if (idx.column() == 0) {
      ::TextMessageNode* n = m_pRecording->d_ptr->m_lNodes[idx.row()];
      switch (role) {
         case Qt::DisplayRole:
            return QVariant(n->m_pMessage->m_PlainText);
         case Qt::DecorationRole         :
            if (n->m_pMessage->direction == Media::Media::Direction::IN)
               return getDelegateManager()->getPixmapManipulationDelegate()->callPhoto(n->m_pContactMethod,QSize(48,48));
            break;
         case (int)Media::TextRecording::Role::Direction            :
            return QVariant::fromValue(n->m_pMessage->direction);
         case (int)Media::TextRecording::Role::AuthorDisplayname    :
            return n->m_pMessage->direction == Media::Media::Direction::IN ?
               n->m_pContactMethod->primaryName() : tr("Me");
         case (int)Media::TextRecording::Role::AuthorUri            :
            return n->m_pContactMethod->uri();
         case (int)Media::TextRecording::Role::AuthorPresenceStatus :
            return n->m_pContactMethod->contact() ?
               n->m_pContactMethod->contact()->isPresent() : n->m_pContactMethod->isPresent();
         case (int)Media::TextRecording::Role::Timestamp            :
            return (int)n->m_pMessage->timestamp;
         case (int)Media::TextRecording::Role::FormattedDate        :
            return QDateTime::fromTime_t(n->m_pMessage->timestamp).toString();
         case (int)Media::TextRecording::Role::IsStatus             :
            return n->m_pMessage->type == Serializable::Message::Type::STATUS;
         case (int)Media::TextRecording::Role::HTML                 :
            return QVariant(n->m_pMessage->m_HTML);
         default:
            break;
      }
   }
   return QVariant();
}

///Number of row
int InstantMessagingModel::rowCount(const QModelIndex& parentIdx) const
{
   Q_UNUSED(parentIdx)
   return m_pRecording->d_ptr->m_lNodes.size();
}

///Model flags
Qt::ItemFlags InstantMessagingModel::flags(const QModelIndex& idx) const
{
   Q_UNUSED(idx)
   return Qt::ItemIsEnabled;
}

///Set model data
bool InstantMessagingModel::setData(const QModelIndex& idx, const QVariant &value, int role)
{
   Q_UNUSED(idx)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}

void InstantMessagingModel::addRowBegin()
{
   const int rc = rowCount();
   beginInsertRows(QModelIndex(),rc,rc);
}

void InstantMessagingModel::addRowEnd()
{
   endInsertRows();
}
