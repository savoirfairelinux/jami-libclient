/****************************************************************************
 *   Copyright (C) 2015-2018 Savoir-faire Linux                               *
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
class NetworkInterfaceModelPrivate;
class Account;

///Static model for handling encryption types
class LIB_EXPORT NetworkInterfaceModel : public QAbstractListModel {
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop

   friend class Account;

public:

   //Model functions
   virtual QVariant      data     ( const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
   virtual int           rowCount ( const QModelIndex& parent = QModelIndex()                ) const override;
   virtual Qt::ItemFlags flags    ( const QModelIndex& index                                 ) const override;
   virtual bool          setData  ( const QModelIndex& index, const QVariant &value, int role)       override;
   virtual QHash<int,QByteArray> roleNames() const override;

   //Getter
   QItemSelectionModel* selectionModel() const;

private:

   //Private constructor, can only be called by 'Account'
   explicit NetworkInterfaceModel(Account* a);
   virtual ~NetworkInterfaceModel();

   NetworkInterfaceModelPrivate* d_ptr;
   Q_DECLARE_PRIVATE(NetworkInterfaceModel)

};
Q_DECLARE_METATYPE(NetworkInterfaceModel*)

