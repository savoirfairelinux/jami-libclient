/****************************************************************************
 *   Copyright (C) 2013-2017 Savoir-faire Linux                          *
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

#include "typedefs.h"
#include <QtCore/QAbstractListModel>

//Qt
class QItemSelectionModel;

//Ring
class Account;
class KeyExchangeModelPrivate;

///Static model for handling encryption types
class LIB_EXPORT KeyExchangeModel : public QAbstractListModel {
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop
   friend class Account;

public:

   enum class Role {
      TYPE = 100
   };

   ///@enum Type Every supported encryption types
   enum class Type {
      NONE = 0,
      SDES = 1,
      COUNT__,
   };

   ///@enum Options Every Key exchange options
   enum class Options {
      RTP_FALLBACK     = 0,
      COUNT__,
   };

   //Private constructor, can only be called by 'Account'
   explicit KeyExchangeModel(Account* account);
   virtual ~KeyExchangeModel();

   //Model functions
   virtual QVariant      data     ( const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
   virtual int           rowCount ( const QModelIndex& parent = QModelIndex()                ) const override;
   virtual bool          setData  ( const QModelIndex& index, const QVariant &value, int role)       override;
   virtual QHash<int,QByteArray> roleNames() const override;

   QItemSelectionModel* selectionModel() const;

private:
   QScopedPointer<KeyExchangeModelPrivate> d_ptr;

public Q_SLOTS:
   void enableSRTP(bool enable);

Q_SIGNALS:
   void srtpEnabled(bool);
};
Q_DECLARE_METATYPE(KeyExchangeModel*)
Q_DECLARE_METATYPE(KeyExchangeModel::Type)

