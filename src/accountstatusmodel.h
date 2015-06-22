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
#ifndef ACCOUNTSTATUSMODEL_H
#define ACCOUNTSTATUSMODEL_H

#include "typedefs.h"
#include <QtCore/QAbstractTableModel>

class AccountStatusModelPrivate;
class Account;

/**This model is used to track registration event. It can be used both
 * for informational purpose like "was my account up at 10:30AM?" or
 * as a generic container to handle various kind of network,
 * configuration or runtime problems.
 */
class LIB_EXPORT AccountStatusModel : public QAbstractTableModel {
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop

   friend class Account;
   friend class AccountPrivate;
   friend class AccountModelPrivate;

public:

   enum class Type {
      REGISTRATION,
      TRANSPORT,
      SIP,
   };

   enum class Columns {
      DESCRIPTION,
      CODE,
      TIME,
      COUNTER,
   };

   //Model functions
   virtual QVariant      data        ( const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
   virtual int           rowCount    ( const QModelIndex& parent = QModelIndex()                ) const override;
   virtual int           columnCount ( const QModelIndex& parent = QModelIndex()                ) const override;
   virtual Qt::ItemFlags flags       ( const QModelIndex& index                                 ) const override;
   virtual bool          setData     ( const QModelIndex& index, const QVariant &value, int role)       override;
   virtual QVariant      headerData  ( int section, Qt::Orientation, int role = Qt::DisplayRole ) const override;
   virtual QHash<int,QByteArray> roleNames() const override;

   //Getter
   QString lastErrorMessage() const;
   int     lastErrorCode   () const;

private:
   //Private mutators
   void addSipRegistrationEvent(const QString& fallbackMessage, int errorCode);
   void addTransportEvent(const QString& description, int code);

   //Private constructor, can only be called by 'Account'
   explicit AccountStatusModel(Account* parent);
   virtual ~AccountStatusModel();

   AccountStatusModelPrivate* d_ptr;
   Q_DECLARE_PRIVATE(AccountStatusModel)
};
Q_DECLARE_METATYPE(AccountStatusModel*)
#endif
