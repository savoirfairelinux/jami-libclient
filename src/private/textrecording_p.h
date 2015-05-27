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
#ifndef TEXTRECORDING_P_H
#define TEXTRECORDING_P_H

//Qt
#include <QtCore/QAbstractListModel>

//Ring
#include <media/media.h>

class SerializableEntityManager;
struct TextMessageNode;
class InstantMessagingModel;
class ContactMethod;

namespace Media {
   class TextRecording;
}

//BEGIN Those classes are serializable to JSon
/**
 * Those classes map 1:1 to the json stored on the disk. References are then
 * extracted, the conversation reconstructed and placed into a TextMessageNode
 * vector.
 */
namespace Serializable {

class Message {
public:
   enum class Type {
      CHAT  , /*!< Normal message between the peer                                           */
      STATUS, /*!< "Room status" message, such as new participants or participants that left */
   };

   ///The time associated with this message
   time_t                  timestamp;
   ///The payload. It will usually be UTF8 text
   QString                 payload  ;
   ///The mimetype associated with the payload
   QString                 mimeType ;
   ///The author display name
   QString                 author   ;
   ///The direction
   Media::Media::Direction direction;
   ///The message Type
   Type type;

   void read (const QJsonObject &json);
   void write(QJsonObject       &json) const;
};


class Group {
public:
   ///The group ID (necessary to untangle the graph
   int id;
   ///All messages from this chunk
   QList<Message*> messages;
   ///If the conversion add new participants, a new file will be created
   QString nextGroupSha1;
   ///This is the group identifier in the file described by `nextGroupSha1`
   int nextGroupId;

   void read (const QJsonObject &json);
   void write(QJsonObject       &json) const;
};

class Peers {
   friend class ::SerializableEntityManager;
public:

   ///The sha1(s) of each participants. If there is onlt one, it should match the filename
   QList<QString> sha1s;
   ///Every message groups associated with this ContactMethod (or ContactMethodGroup)
   QList<Group*> groups;

   ///This attribute store if the file has changed
   bool hasChanged;

   void read (const QJsonObject &json);
   void write(QJsonObject       &json) const;

private:
   Peers() : hasChanged(false) {}
};

}
//END Those classes are serializable to JSon

namespace Media {

/**
 * The Media::Recording private class. This is where the reconstructed
 * conversation is stored. This class is also used as backend for the
 * IM Model. The messages themselves are added by the Media::Text.
 */
class TextRecordingPrivate {
public:
   TextRecordingPrivate(TextRecording* r);

   //Attributes
   InstantMessagingModel*      m_pImModel        ;
   QVector<::TextMessageNode*> m_lNodes          ;
   Serializable::Group*        m_pCurrentGroup   ;
   QList<Serializable::Peers*> m_lAssociatedPeers;

   //Helper
   void insertNewMessage(const QString& message, const ContactMethod* cm, Media::Media::Direction direction);
   QHash<QByteArray,QByteArray> toJsons() const;

private:
   TextRecording* q_ptr;
};

} //Media::


/**
 * This class ensure that only one Serializable::Peer structure exist for each
 * peer [group].
 */
class SerializableEntityManager
{
public:
   static Serializable::Peers* peer(const ContactMethod* cm);
   static Serializable::Peers* peers(QList<const ContactMethod*> cms);
   static Serializable::Peers* fromSha1(const QByteArray& sha1);
   static Serializable::Peers* fromJson(const QJsonObject& obj);
private:
   static QHash<QByteArray,Serializable::Peers*> m_hPeers;
};

/**
 * This is the structure used internally to create the text conversation
 * frontend. It will be stored as a vector by the IM Model but also implement
 * a chained list for convenience
 */
struct TextMessageNode
{
   TextMessageNode() : m_pNext(nullptr),m_pContactMethod(nullptr)
   {}

   Serializable::Message* m_pMessage      ;
   ContactMethod*         m_pContactMethod;
   TextMessageNode*       m_pNext         ;
};

///Model for the Instant Messaging (IM) features
class InstantMessagingModel : public QAbstractListModel
{
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop

public:

   //Constructor
   explicit InstantMessagingModel(Media::TextRecording*);
   virtual ~InstantMessagingModel();

   //Abstract model function
   virtual QVariant      data     ( const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
   virtual int           rowCount ( const QModelIndex& parent = QModelIndex()                ) const override;
   virtual Qt::ItemFlags flags    ( const QModelIndex& index                                 ) const override;
   virtual bool  setData  ( const QModelIndex& index, const QVariant &value, int role)       override;
   virtual QHash<int,QByteArray> roleNames() const override;

   //Attributes
   Media::TextRecording* m_pRecording;

   //Helper
   void addRowBegin();
   void addRowEnd();
};

#endif
