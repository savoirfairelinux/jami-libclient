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

   /// @enum CodecModel::Action Manage a CodecModel lifecycle
   enum class EditAction {
      SAVE   = 0, /*!< Save the model, if there is a conflict, use "ours"          */
      MODIFY = 1, /*!< Notify the state machine that the data has changed          */
      RELOAD = 2, /*!< Reload from the daemon, if there is a conflict, use "their" */
      CLEAR  = 3, /*!< Remove all codecs                                           */
      COUNT__
   };

   /// @enum CodecModel::EditState track the changes from both clients and daemon
   enum class EditState {
      LOADING  = 0, /*!< The codecs are being loaded, they are not ready yet       */
      READY    = 1, /*!< Both side are synchronized                                */
      MODIFIED = 2, /*!< Our version differ from the remote one                    */
      OUTDATED = 3, /*!< The remote version differ from ours                       */
      COUNT__
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

   //Mutator
   bool performAction(CodecModel::EditAction action);

   //Getter
   int                   acceptedPayloadTypes() const;
   QItemSelectionModel*  selectionModel      () const;
   CodecModel::EditState editState           () const;

   //Operator
   CodecModel* operator<<(CodecModel::EditAction& action);

public Q_SLOTS:
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

CodecModel* LIB_EXPORT operator<<(CodecModel* a, CodecModel::EditAction action);

#endif
