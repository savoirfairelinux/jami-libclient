/****************************************************************************
 *   Copyright (C) 2013-2018 Savoir-faire Linux                          *
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

#include <QtCore/QAbstractTableModel>
#include "typedefs.h"
#include "phonedirectorymodel.h"
#include <itemdataroles.h>

//Qt
class QItemSelectionModel;

//Ring
class ContactMethod;
class Call;

//Private
class NumberCompletionModelPrivate;

class LIB_EXPORT NumberCompletionModel : public QAbstractTableModel {
   Q_OBJECT

public:

   //Properties
   Q_PROPERTY(QString prefix READ prefix)
   Q_PROPERTY(bool displayMostUsedNumbers READ displayMostUsedNumbers WRITE setDisplayMostUsedNumbers)

   enum Role {
      ALTERNATE_ACCOUNT= (int)Ring::Role::UserRole,
      FORCE_ACCOUNT,
      ACCOUNT      ,
      PEER_NAME    ,
   };

   NumberCompletionModel();
   virtual ~NumberCompletionModel();

   //Abstract model member
   virtual QVariant      data       ( const QModelIndex& index, int role = Qt::DisplayRole                 ) const override;
   virtual int           rowCount   ( const QModelIndex& parent = QModelIndex()                            ) const override;
   virtual Qt::ItemFlags flags      ( const QModelIndex& index                                             ) const override;
   virtual bool          setData    ( const QModelIndex& index, const QVariant &value, int role            )       override;
   virtual int           columnCount( const QModelIndex& parent = QModelIndex()                            ) const override;
   virtual QVariant      headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

   virtual QHash<int,QByteArray> roleNames() const override;

   //Setters
   void setCall(Call* call);
   void setUseUnregisteredAccounts(bool value);
   void setDisplayMostUsedNumbers(bool value);

   //Getters
   Call* call() const;
   ContactMethod* number(const QModelIndex& idx) const;
   bool isUsingUnregisteredAccounts();
   QString prefix() const;
   bool displayMostUsedNumbers() const;
   QItemSelectionModel* selectionModel() const;

private:
   NumberCompletionModelPrivate* d_ptr;
   Q_DECLARE_PRIVATE(NumberCompletionModel)

public Q_SLOTS:
   bool callSelectedNumber();

Q_SIGNALS:
   void enabled(bool);

};

