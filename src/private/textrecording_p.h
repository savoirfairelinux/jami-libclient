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

class SerializableEntityManager;

//BEGIN Those classes are serializable to JSon
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
   QList<Message> messages;
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
   QList<Group> groups;

   void read (const QJsonObject &json);
   void write(QJsonObject       &json) const;

private:
   Peers() {}
};

}
//END Those classes are serializable to JSon

namespace Media {

struct TextMessageNode;

class TextRecordingPrivate {
public:
   TextRecordingPrivate(TextRecording* r);

   //Attributes
   InstantMessagingModel*    m_pImModel     ;
   QVector<TextMessageNode*> m_lNodes       ;
   Serializable::Group*      m_pCurrentGroup;

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
   static Serializable::Peers* peer(ContactMethod* cm);
   static Serializable::Peers* peers(QList<ContactMethod*> cms);
private:
   static QHash<QByteArray,Serializable::Peers*> m_hPeers;
};

struct TextMessageNode
{
   TextMessageNode() : m_pNext(nullptr),m_pContactMethod(nullptr)
   {}

   Serializable::Message* m_pMessage      ;
   ContactMethod*         m_pContactMethod;
   TextMessageNode*       m_pNext         ;
};

#endif
