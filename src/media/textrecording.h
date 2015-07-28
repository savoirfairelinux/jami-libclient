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
#ifndef MEDIA_TEXTRECORDING_H
#define MEDIA_TEXTRECORDING_H

#include <media/recording.h>
#include <QtCore/QSortFilterProxyModel>

//Qt
class QJsonObject;
class QAbstractListModel;

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

   ///ProxyModel to filter message by timestamp
   class LIB_EXPORT InstantMessagingProxyModel final : public QSortFilterProxyModel
   {

   public:
     explicit InstantMessagingProxyModel(QAbstractItemModel *parent = 0);
     virtual void setFilterMinTimestamp(const time_t& min);
     virtual void setFilterMaxTimestamp(const time_t& max);

   protected:
     //virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
     virtual bool filterAcceptsRow(int sourceRow,const QModelIndex &sourceParent) const override;

   private:
     time_t minTimestamp;
     time_t maxTimestamp;
   };

public:

   enum class Role {
      Direction            = Qt::UserRole +1,
      AuthorDisplayname                     ,
      AuthorUri                             ,
      AuthorPresenceStatus                  ,
      Timestamp                             ,
      FormattedDate                         ,
      IsStatus                              ,
      HTML                                  ,
   };

   //Constructor
   explicit TextRecording();
   virtual ~TextRecording();
   static TextRecording* fromJson(const QList<QJsonObject>& items, const ContactMethod* cm = nullptr);

   //Getter
   QAbstractListModel* instantMessagingModel() const;
   InstantMessagingProxyModel* instantMessagingProxyModel();
private:
   TextRecordingPrivate* d_ptr;
   InstantMessagingProxyModel* m_ImProxModel;
   Q_DECLARE_PRIVATE(TextRecording)
};

}

#endif
