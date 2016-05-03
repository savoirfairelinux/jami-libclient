/****************************************************************************
 *   Copyright (C) 2015-2016 by Savoir-faire Linux                               *
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
#pragma once

#include <media/recording.h>
#include <media/media.h>
#include <itemdataroles.h>

//Qt
class QJsonObject;
class QAbstractItemModel;

//Ring
class IMConversationManagerPrivate;
class LocalTextRecordingEditor;
class ContactMethod;
class InstantMessagingModel;

namespace Media {

class TextRecordingPrivate;
class Text;

class LIB_EXPORT TextRecording : public Recording
{
   Q_OBJECT

   //InstantMessagingModel is a view on top of TextRecording data
   friend class ::InstantMessagingModel;
   friend class ::IMConversationManagerPrivate;
   friend class ::LocalTextRecordingEditor;
   friend class Text;
   friend class ::ContactMethod;

public:

   enum class Role {
      Direction            = static_cast<int>(Ring::Role::UserRole) + 1,
      AuthorDisplayname    ,
      AuthorUri            ,
      AuthorPresenceStatus ,
      Timestamp            ,
      IsRead               ,
      FormattedDate        ,
      IsStatus             ,
      HTML                 ,
      HasText              ,
      ContactMethod        ,
      DeliveryStatus       ,
      FormattedHtml        ,
   };

   //Constructor
   explicit TextRecording();
   virtual ~TextRecording();
   static TextRecording* fromJson(const QList<QJsonObject>& items, const ContactMethod* cm = nullptr);

   //Getter
   QAbstractItemModel* instantMessagingModel    (                         ) const;
   QAbstractItemModel* instantTextMessagingModel(                         ) const;
   QAbstractItemModel* unreadInstantTextMessagingModel(                   ) const;
   bool                isEmpty                  (                         ) const;
   bool                hasMimeType              ( const QString& mimeType ) const;
   QStringList         mimeTypes                (                         ) const;
   QVector<ContactMethod*> peers                (                         ) const;

   //Helper
   void setAllRead();

Q_SIGNALS:
   void messageInserted(const QMap<QString,QString>& message, ContactMethod* cm, Media::Media::Direction direction);
   void unreadCountChange(int count);

private:
   TextRecordingPrivate* d_ptr;
   Q_DECLARE_PRIVATE(TextRecording)
};

}
