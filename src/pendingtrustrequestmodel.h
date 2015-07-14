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
#ifndef PENDINGTRUSTREQUESTMODEL_H
#define PENDINGTRUSTREQUESTMODEL_H

#include <QtCore/QAbstractTableModel>
#include <typedefs.h>

class Account;
class PendingTrustRequestModelPrivate;
class TrustRequest;

/**
 * List the pending (incoming) trust request for an account
 */
class LIB_EXPORT PendingTrustRequestModel : public QAbstractTableModel
{
   Q_OBJECT

   friend class Account;
   friend class AccountPrivate;
   friend class AccountModelPrivate;
public:

   //Model functions
   virtual QVariant      data        ( const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
   virtual int           rowCount    ( const QModelIndex& parent = QModelIndex()                ) const override;
   virtual int           columnCount ( const QModelIndex& parent = QModelIndex()                ) const override;
   virtual Qt::ItemFlags flags       ( const QModelIndex& index                                 ) const override;
   virtual bool          setData     ( const QModelIndex& index, const QVariant &value, int role)       override;
   virtual QHash<int,QByteArray> roleNames() const override;

private:
   explicit PendingTrustRequestModel(Account* a);
   virtual ~PendingTrustRequestModel();

   PendingTrustRequestModelPrivate* d_ptr;
   Q_DECLARE_PRIVATE(PendingTrustRequestModel)

Q_SIGNALS:
   void requestAccepted (TrustRequest* r);
   void requestDiscarded(TrustRequest* r);
};

#endif
