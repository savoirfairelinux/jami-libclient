/****************************************************************************
 *   Copyright (C) 2012-2015 by Savoir-Faire Linux                          *
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
#ifndef CODEC_MODEL_H
#define CODEC_MODEL_H

#include <QtCore/QAbstractListModel>

//Qt
#include <QtCore/QString>
class QSortFilterProxyModel;
class QItemSelectionModel;

//Ring
#include <typedefs.h>

class Account;
class CodecModelPrivate;

///AudioCodecModel: A model for account audio codec
class LIB_EXPORT CodecModel : public QAbstractListModel {
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop

   //Only Account:: can create this type
   friend class Account;

public:

   //Roles
   enum Role {
      ID         = 103,
      NAME       = 100,
      BITRATE    = 101,
      SAMPLERATE = 102,
      TYPE       = 104,
   };

   //Properties
   Q_PROPERTY(QSortFilterProxyModel* audioCodecs    READ audioCodecs    )
   Q_PROPERTY(QSortFilterProxyModel* videoCodecs    READ videoCodecs    )
   Q_PROPERTY(QItemSelectionModel*   selectionModel READ selectionModel )

   //Abstract model member
   virtual QVariant              data        ( const QModelIndex& index, int role = Qt::DisplayRole      ) const override;
   virtual int                   rowCount    ( const QModelIndex& parent = QModelIndex()                 ) const override;
   virtual Qt::ItemFlags         flags       ( const QModelIndex& index                                  ) const override;
   virtual bool                  setData     ( const QModelIndex& index, const QVariant &value, int role )       override;
   virtual QHash<int,QByteArray> roleNames   (                                                           ) const override;
   virtual QMimeData*            mimeData    ( const QModelIndexList &indexes                            ) const override;
   virtual QStringList           mimeTypes   (                                                           ) const override;
   virtual bool                  dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
   virtual Qt::DropActions       supportedDragActions() const override;
   virtual Qt::DropActions       supportedDropActions() const override;

   //Proxies
   QSortFilterProxyModel* audioCodecs() const;
   QSortFilterProxyModel* videoCodecs() const;

   //Getter
   int                  acceptedPayloadTypes() const;
   QItemSelectionModel* selectionModel      () const;

public Q_SLOTS:
   QModelIndex add         (                        );
   void        remove      ( const QModelIndex& idx );
   void        clear       (                        );
   void        reload      (                        );
   void        save        (                        );
   bool        moveUp      (                        );
   bool        moveDown    (                        );

private:

   //Constructor
   explicit CodecModel(Account* account);
   virtual ~CodecModel();

   QScopedPointer<CodecModelPrivate> d_ptr;
   Q_DECLARE_PRIVATE(CodecModel)
};

Q_DECLARE_METATYPE(CodecModel*)

#endif
