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
#include "textrecording.h"

//Qt
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonValue>
#include <QtCore/QJsonDocument>
#include <QtCore/QDateTime>
#include <QtCore/QCryptographicHash>
#include <QtCore/QUrl>

//Daemon
#include <account_const.h>

//Ring
#include "callmodel.h"
#include "contactmethod.h"
#include "account.h"
#include "phonedirectorymodel.h"
#include "accountmodel.h"
#include "personmodel.h"
#include "private/textrecording_p.h"
#include "globalinstances.h"
#include "interfaces/pixmapmanipulatori.h"
#include "itemdataroles.h"
#include "mime.h"
#include "dbus/configurationmanager.h"

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

Media::TextRecordingPrivate::TextRecordingPrivate(TextRecording* r) : q_ptr(r),m_pImModel(nullptr),m_pCurrentGroup(nullptr),m_UnreadCount(0)
{
}

Media::TextRecording::TextRecording() : Recording(Recording::Type::TEXT), d_ptr(new TextRecordingPrivate(this))
{
}

Media::TextRecording::~TextRecording()
{
   delete d_ptr;
}

/**
 * Updates the message status and potentially the message id, if a new status is set.
 * Returns true if the Message object was modified, false otherwise.
 */
bool Media::TextRecordingPrivate::updateMessageStatus(Serializable::Message* m, TextRecording::Status newSatus)
{
    bool modified = false;

    if (static_cast<int>(newSatus) >= static_cast<int>(TextRecording::Status::COUNT__)) {
        qWarning() << "Unknown message status with code: " << static_cast<int>(newSatus);
        newSatus = TextRecording::Status::UNKNOWN;
    } else {
        //READ status is not used yet it'll be the final state when it is
        if (newSatus == TextRecording::Status::READ
                || newSatus == TextRecording::Status::SENT
                || newSatus == TextRecording::Status::FAILURE) {
            m_hPendingMessages.remove(m->id);
            if (m->id != 0) {
                m->id = 0;
                modified = true;
            }
        }
    }

    if (m->deliveryStatus != newSatus) {
        m->deliveryStatus = newSatus;
        modified = true;
    }
    return modified;
}

void Media::TextRecordingPrivate::accountMessageStatusChanged(const uint64_t id, DRing::Account::MessageStates status)
{
    if (auto node = m_hPendingMessages.value(id, nullptr)) {
        if (updateMessageStatus(node->m_pMessage, static_cast<TextRecording::Status>(status))) {
            //You're looking at why local file storage is a "bad" idea
            q_ptr->save();

            auto msg_index = m_pImModel->index(node->m_row, 0);
            m_pImModel->dataChanged(msg_index, msg_index);
        }
    }
}

bool Media::TextRecording::hasMimeType(const QString& mimeType) const
{
   return d_ptr->m_hMimeTypes.contains(mimeType);
}

QStringList Media::TextRecording::mimeTypes() const
{
   return d_ptr->m_lMimeTypes;
}

///Get the instant messaging model associated with this recording
QAbstractItemModel* Media::TextRecording::instantMessagingModel() const
{
   if (!d_ptr->m_pImModel) {
      d_ptr->m_pImModel = new InstantMessagingModel(const_cast<TextRecording*>(this));
   }

   return d_ptr->m_pImModel;
}

///Set all messages as read and then save the recording
void Media::TextRecording::setAllRead()
{
    bool changed = false;
    for(int row = 0; row < d_ptr->m_lNodes.size(); ++row) {
        if (!d_ptr->m_lNodes[row]->m_pMessage->isRead) {
            d_ptr->m_lNodes[row]->m_pMessage->isRead = true;
            if (d_ptr->m_pImModel) {
                auto idx = d_ptr->m_pImModel->index(row, 0);
                emit d_ptr->m_pImModel->dataChanged(idx,idx);
            }
            changed = true;
        }
    }
    if (changed) {
        // TODO: we assume that the CM is the same for now, and that at least some of the messages
        //       are text
        int oldVal = d_ptr->m_UnreadCount;
        d_ptr->m_UnreadCount = 0;
        emit unreadCountChange(-oldVal);
        emit d_ptr->m_lNodes[0]->m_pContactMethod->unreadTextMessageCountChanged();
        emit d_ptr->m_lNodes[0]->m_pContactMethod->changed();
        save();
    }
}

QVector<ContactMethod*> Media::TextRecording::peers() const
{
    QVector<ContactMethod*> cms;

    for (const Serializable::Peers* peers : d_ptr->m_lAssociatedPeers) {
        for (const Serializable::Peer* peer : peers->peers) {
            cms << peer->m_pContactMethod;
        }
    }

    return cms;
}

/**
 * I (Emmanuel Lepage) is in the process of writing a better one for this that
 * can be upstreamed into Qt (there is interest in merging a generic QVariant
   filter model), however, it is too complex to merge into LRC for such basic
   use case. So, for the sake of simplicity until upstream have this feature,
   here is a subset of the generic filter proxy. The time between now and the
   Qt review + Qt release + LRC drop old version of Qt is too long anyway.

   A copy of the code (copyrighted by me) is available in the ring-kde
   next release for those interested. In 2016-2017, this code could probably
   be replaced by the new one, be it in KItemModels (the KDE abstract proxy
   library) or QtCore.
 */
class TextProxyModel : public QSortFilterProxyModel
{
public:
   explicit TextProxyModel(QObject* parent) : QSortFilterProxyModel(parent){}
   virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override
   {
      const QModelIndex srcIdx = sourceModel()->index(source_row, filterKeyColumn(), source_parent);

      return srcIdx.data((int)Media::TextRecording::Role::HasText).toBool();
   }
};

/**
 * Subset of the instantMessagingModel() with only plain text and HTML
 * messages. This model can be displayed directly to the user.
 */
QAbstractItemModel* Media::TextRecording::instantTextMessagingModel() const
{
   if (!d_ptr->m_pTextMessagesModel) {
      auto p = new TextProxyModel(const_cast<TextRecording*>(this));
      p->setSourceModel(instantMessagingModel());
      d_ptr->m_pTextMessagesModel = p;
   }

   return d_ptr->m_pTextMessagesModel;
}

/**
 * Proxy model to get the unread text messages, as well as their number (rowCount)
 */
class UnreadProxyModel : public QSortFilterProxyModel
{
public:
    explicit UnreadProxyModel(QObject* parent) : QSortFilterProxyModel(parent){}
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override
    {
        const QModelIndex srcIdx = sourceModel()->index(source_row, filterKeyColumn(), source_parent);

        return !srcIdx.data((int)Media::TextRecording::Role::IsRead).toBool();
    }
};

/**
 * Subset of the instantTextMessagingModel() with only unread plain text and HTML
 * messages. This model can be used to get the number of unread messages.
 */
QAbstractItemModel* Media::TextRecording::unreadInstantTextMessagingModel() const
{
    if (!d_ptr->m_pUnreadTextMessagesModel) {
       auto p = new UnreadProxyModel(instantTextMessagingModel());
       p->setSourceModel(instantTextMessagingModel());
       d_ptr->m_pUnreadTextMessagesModel = p;
    }

    return d_ptr->m_pUnreadTextMessagesModel;
}


bool Media::TextRecording::isEmpty() const
{
   return !size();
}

int Media::TextRecording::size() const
{
    return d_ptr->m_lNodes.size();
}

// Qt convention compat
int Media::TextRecording::count() const { return size(); }

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

Media::TextRecording* Media::TextRecording::fromJson(const QList<QJsonObject>& items, const ContactMethod* cm, CollectionInterface* backend)
{
    TextRecording* t = new TextRecording();
    if (backend)
        t->setCollection(backend);

    ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();

    //Load the history data
    for (const QJsonObject& obj : items) {
        Serializable::Peers* p = SerializableEntityManager::fromJson(obj,cm);
        t->d_ptr->m_lAssociatedPeers << p;
    }

    //Create the model
    bool statusChanged = false; // if a msg status changed during parsing, we need to re-save the model
    t->instantMessagingModel();

    //Reconstruct the conversation
    //TODO do it right, right now it flatten the graph
    for (const Serializable::Peers* p : t->d_ptr->m_lAssociatedPeers) {
        //Seems old version didn't store that
        if (p->peers.isEmpty())
            continue;
        // TODO: for now assume the convo is with only 1 CM at a time
        auto peerCM = p->peers.at(0)->m_pContactMethod;

        // get the latest timestamp to set last used
        time_t lastUsed = 0;
        for (const Serializable::Group* g : p->groups) {
            for (Serializable::Message* m : g->messages) {
                ::TextMessageNode* n  = new ::TextMessageNode();
                n->m_pMessage         = m                      ;
                if (!n->m_pMessage->contactMethod) {
                    if (cm) {
                        n->m_pMessage->contactMethod = const_cast<ContactMethod*>(cm); //TODO remove in 2016
                        n->m_pMessage->authorSha1 = cm->sha1();

                        if (p->peers.isEmpty())
                            addPeer(const_cast<Serializable::Peers*>(p), cm);
                    } else {
                        if (p->m_hSha1.contains(n->m_pMessage->authorSha1)) {
                            n->m_pMessage->contactMethod = p->m_hSha1[n->m_pMessage->authorSha1];
                        } else {
                            // message was outgoing and author sha1 was set to that of the sending account
                            n->m_pMessage->contactMethod = peerCM;
                            n->m_pMessage->authorSha1 = peerCM->sha1();
                        }
                    }
                }
                n->m_pContactMethod   = m->contactMethod;
                t->d_ptr->m_pImModel->addRowBegin();
                t->d_ptr->m_lNodes << n;
                t->d_ptr->m_pImModel->addRowEnd();

                if (lastUsed < n->m_pMessage->timestamp)
                    lastUsed = n->m_pMessage->timestamp;
                if (m->id) {
                    int status = configurationManager.getMessageStatus(m->id);
                    t->d_ptr->m_hPendingMessages[m->id] = n;
                    if (t->d_ptr->updateMessageStatus(m, static_cast<TextRecording::Status>(status)))
                        statusChanged = true;
                }
            }
        }

        if (statusChanged)
            t->save();

        // update the timestamp of the CM
        peerCM->setLastUsed(lastUsed);
    }

    return t;
}

void Media::TextRecordingPrivate::insertNewMessage(const QMap<QString,QString>& message, ContactMethod* cm, Media::Media::Direction direction, uint64_t id)
{
    //Only create it if none was found on the disk
    if (!m_pCurrentGroup) {
        m_pCurrentGroup = new Serializable::Group();

        auto cMethod = q_ptr->call() ? q_ptr->call()->peerContactMethod() : cm;

        Serializable::Peers* p = SerializableEntityManager::peer(cMethod);

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
   m->id = id;

   if (direction == Media::Media::Direction::OUT)
      m->isRead = true; // assume outgoing messages are read, since we're sending them

   static const int profileSize = QString(RingMimes::PROFILE_VCF).size();

   QMapIterator<QString, QString> iter(message);
   while (iter.hasNext()) {
      iter.next();
      if (iter.key().left(profileSize) == RingMimes::PROFILE_VCF)
          return;
      if (iter.value() != QLatin1String("application/resource-lists+xml")) { //This one is useless
         const QString mimeType = iter.key();

         Serializable::Payload* p = new Serializable::Payload();
         p->mimeType = mimeType    ;
         p->payload  = iter.value();
         m->payloads << p;

         if (p->mimeType == QLatin1String("text/plain")) {
            m->m_PlainText = p->payload;
            m->m_HasText   = true;
         }
         else if (p->mimeType == QLatin1String("text/html")) {
            m->m_HTML    = p->payload;
            m->m_HasText = true;
         }

         // Make the clients life easier and tell the payload type
         const int hasArgs = mimeType.indexOf(';');
         const QString strippedMimeType = hasArgs != -1 ? mimeType.left(hasArgs) : mimeType;
         const int currentSize = m_hMimeTypes.size();

         m_hMimeTypes[strippedMimeType] = true;

         if (currentSize != m_hMimeTypes.size())
            m_lMimeTypes << strippedMimeType;
      }
   }
   m_pCurrentGroup->messages << m;

   //Make sure the model exist
   q_ptr->instantMessagingModel();

   //Update the reconstructed conversation
   ::TextMessageNode* n  = new ::TextMessageNode()       ;
   n->m_pMessage         = m                             ;
   n->m_pContactMethod   = const_cast<ContactMethod*>(cm);
   n->m_row              = m_pImModel->rowCount();
   m_pImModel->addRowBegin();
   m_lNodes << n;
   m_pImModel->addRowEnd();

   if (m->id > 0)
       m_hPendingMessages[id] = n;

   //Save the conversation
   q_ptr->save();

   cm->setLastUsed(currentTime);
   emit q_ptr->messageInserted(message, const_cast<ContactMethod*>(cm), direction);
   if (!m->isRead) {
      m_UnreadCount += 1;
      emit q_ptr->unreadCountChange(1);
      emit cm->unreadTextMessageCountChanged();
      emit cm->changed();
   }
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
   id         = json["id"        ].toVariant().value<uint64_t>(                     );
   deliveryStatus = static_cast<Media::TextRecording::Status>(json["deliveryStatus"].toInt());

   QJsonArray a = json["payloads"].toArray();
   for (int i = 0; i < a.size(); ++i) {
      QJsonObject o = a[i].toObject();
      Payload* p = new Payload();
      p->read(o);
      payloads << p;

      if (p->mimeType == "text/plain") {
         m_PlainText = p->payload;
         m_HasText   = true;
      }
      else if (p->mimeType == "text/html") {
         m_HTML    = p->payload;
         m_HasText = true;
      }
   }

   //Load older conversation from a time when only 1 mime/payload pair was supported
   if (!json["payload"   ].toString().isEmpty()) {
      Payload* p  = new Payload();
      p->payload  = json["payload"  ].toString();
      p->mimeType = json["mimeType" ].toString();
      payloads << p;
      m_PlainText = p->payload;
      m_HasText   = true;
   }
}

void Serializable::Message::write(QJsonObject &json) const
{
   json["timestamp"  ] = static_cast<int>(timestamp);
   json["authorSha1" ] = authorSha1                 ;
   json["direction"  ] = static_cast<int>(direction);
   json["type"       ] = static_cast<int>(type)     ;
   json["isRead"     ] = isRead                     ;
   json["id"         ] = QString::number(id);
   json["deliveryStatus"         ] = static_cast<int>(deliveryStatus);

   QJsonArray a;
   foreach (const Payload* p, payloads) {
      QJsonObject o;
      p->write(o);
      a.append(o);
   }
   json["payloads"] = a;
}

const QRegularExpression Serializable::Message::m_linkRegex = QRegularExpression(QStringLiteral("((?>(?>https|http|ftp|ring):|www\\.)(?>[^\\s,.);!>]|[,.);!>](?!\\s|$))+)"),
                                                                                 QRegularExpression::CaseInsensitiveOption);

const QString& Serializable::Message::getFormattedHtml()
{
    if (m_FormattedHtml.isEmpty())
    {
        QString re;
        auto p = 0;
        auto it = m_linkRegex.globalMatch(m_PlainText);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            auto start = match.capturedStart();

            auto url = QUrl::fromUserInput(match.capturedRef().toString());

            if (start > p)
                re.append(m_PlainText.mid(p, start - p).toHtmlEscaped().replace(QLatin1Char('\n'),
                                                                                QStringLiteral("<br/>")));
            re.append(QStringLiteral("<a href=\"%1\">%2</a>")
                      .arg(QString::fromLatin1(url.toEncoded()).toHtmlEscaped(),
                           match.capturedRef().toString().toHtmlEscaped()));
            m_LinkList.append(url);
            p = match.capturedEnd();
        }
        if (p < m_PlainText.size())
            re.append(m_PlainText.mid(p, m_PlainText.size() - p).toHtmlEscaped());

        m_FormattedHtml = QString("<body>%1</body>").arg(re);
    }
    return m_FormattedHtml;
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

   Account* a     = AccountModel::instance().getById(accountId.toLatin1());
   Person* person = personUID.isEmpty() ?
      nullptr : PersonModel::instance().getPersonByUid(personUID.toLatin1());

   m_pContactMethod = PhoneDirectoryModel::instance().getNumber(uri,person,a);
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
      peer->read(o);
      m_hSha1[peer->sha1] = peer->m_pContactMethod;
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
      roles.insert((int)Media::TextRecording::Role::IsRead              , "isRead"              );
      roles.insert((int)Media::TextRecording::Role::FormattedDate       , "formattedDate"       );
      roles.insert((int)Media::TextRecording::Role::IsStatus            , "isStatus"            );
      roles.insert((int)Media::TextRecording::Role::DeliveryStatus      , "deliveryStatus"      );
      roles.insert((int)Media::TextRecording::Role::FormattedHtml       , "formattedHtml"       );
      roles.insert((int)Media::TextRecording::Role::LinkList            , "linkList"            );
      roles.insert((int)Media::TextRecording::Role::Id                  , "id"                  );
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
               return GlobalInstances::pixmapManipulator().decorationRole(n->m_pContactMethod);
            else if (m_pRecording->call() && m_pRecording->call()->account()
              && m_pRecording->call()->account()->contactMethod()->contact()) {
               auto cm = m_pRecording->call()->account()->contactMethod();
               return GlobalInstances::pixmapManipulator().decorationRole(cm);
            } else if (n->m_pMessage->direction == Media::Media::Direction::OUT && n->m_pContactMethod->account()){
                return GlobalInstances::pixmapManipulator().decorationRole(n->m_pContactMethod->account());
            } else {
                /* It's most likely an account that doesn't exist anymore
                 * Use a fallback image in pixmapManipulator
                */
                return GlobalInstances::pixmapManipulator().decorationRole((ContactMethod*)nullptr);
            }
            break;
         case (int)Media::TextRecording::Role::Direction            :
            return QVariant::fromValue(n->m_pMessage->direction);
         case (int)Media::TextRecording::Role::AuthorDisplayname    :
         case (int)Ring::Role::Name                                 :
            if (n->m_pMessage->direction == Media::Media::Direction::IN)
               return n->m_pContactMethod->roleData(static_cast<int>(Ring::Role::Name));
            else
               return tr("Me");
         case (int)Media::TextRecording::Role::AuthorUri            :
         case (int)Ring::Role::Number                               :
            return n->m_pContactMethod->uri();
         case (int)Media::TextRecording::Role::AuthorPresenceStatus :
            // Always consider "self" as present
            if (n->m_pMessage->direction == Media::Media::Direction::OUT)
               return true;
            else
               return n->m_pContactMethod->contact() ?
                  n->m_pContactMethod->contact()->isPresent() : n->m_pContactMethod->isPresent();
         case (int)Media::TextRecording::Role::Timestamp            :
            return (uint)n->m_pMessage->timestamp;
         case (int)Media::TextRecording::Role::IsRead               :
            return (int)n->m_pMessage->isRead;
         case (int)Media::TextRecording::Role::FormattedDate        :
            return QDateTime::fromTime_t(n->m_pMessage->timestamp).toString();
         case (int)Media::TextRecording::Role::IsStatus             :
            return n->m_pMessage->type == Serializable::Message::Type::STATUS;
         case (int)Media::TextRecording::Role::HTML                 :
            return QVariant(n->m_pMessage->m_HTML);
         case (int)Media::TextRecording::Role::HasText              :
            return n->m_pMessage->m_HasText;
         case (int)Media::TextRecording::Role::ContactMethod        :
            return QVariant::fromValue(n->m_pContactMethod);
         case (int)Media::TextRecording::Role::DeliveryStatus       :
            return QVariant::fromValue(n->m_pMessage->deliveryStatus);
         case (int)Media::TextRecording::Role::FormattedHtml        :
            return QVariant::fromValue(n->m_pMessage->getFormattedHtml());
         case (int)Media::TextRecording::Role::LinkList             :
            return QVariant::fromValue(n->m_pMessage->m_LinkList);
         case (int)Media::TextRecording::Role::Id                   :
            return QVariant::fromValue(n->m_pMessage->id);
         default:
            break;
      }
   }
   return QVariant();
}

///Number of row
int InstantMessagingModel::rowCount(const QModelIndex& parentIdx) const
{
   if (!parentIdx.isValid())
      return m_pRecording->d_ptr->m_lNodes.size();
   return 0;
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
    if (idx.column() || !idx.isValid())
        return false;

    bool changed = false;

    ::TextMessageNode* n = m_pRecording->d_ptr->m_lNodes[idx.row()];
    switch (role) {
        case (int)Media::TextRecording::Role::IsRead               :
            if (n->m_pMessage->isRead != value.toBool()) {
                n->m_pMessage->isRead = value.toBool();
                if (n->m_pMessage->m_HasText) {
                    int val = value.toBool() ? -1 : +1;
                    m_pRecording->d_ptr->m_UnreadCount += val;
                    emit m_pRecording->unreadCountChange(val);
                    emit n->m_pContactMethod->unreadTextMessageCountChanged();
                    emit n->m_pContactMethod->changed();
                }
                emit dataChanged(idx,idx);
                changed = true;
            }
            break;
        default:
            return false;
    }

    //Save the conversation
    if (changed)
        m_pRecording->save();
    return true;
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

void Media::TextRecordingPrivate::clear()
{
    m_pImModel->clear();

    if (m_UnreadCount != 0) {
        m_UnreadCount = 0;
        emit q_ptr->unreadCountChange(0);
    }
}

void InstantMessagingModel::clear()
{
    beginResetModel();

    for ( TextMessageNode *node : m_pRecording->d_ptr->m_lNodes) {
        for (Serializable::Payload *payload : node->m_pMessage->payloads) {
            delete payload;
        }
        delete node->m_pMessage;
        delete node;
    }
    m_pRecording->d_ptr->m_lNodes.clear();

    for (Serializable::Peers *peers : m_pRecording->d_ptr->m_lAssociatedPeers) {
        for (Serializable::Group *group : peers->groups) {
            group->messages.clear();
        }
    }
    m_pRecording->d_ptr->m_lAssociatedPeers.clear();

    //TODO: holly memory leaks batman! what else do we need to delete?

    m_pRecording->d_ptr->m_pCurrentGroup = nullptr;
    m_pRecording->d_ptr->m_hMimeTypes.clear();
    m_pRecording->d_ptr->m_lMimeTypes.clear();

    endResetModel();
}
