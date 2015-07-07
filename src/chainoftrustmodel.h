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
#ifndef CHAINOFTRUSTMODEL_H
#define CHAINOFTRUSTMODEL_H

#include <QtCore/QAbstractItemModel>
#include <typedefs.h>

class ChainOfTrustModelPrivate;
class Certificate;

class LIB_EXPORT ChainOfTrustModel :  public QAbstractItemModel
{
   Q_OBJECT

   friend class Certificate;
public:

   enum class Role {
      OBJECT = Qt::UserRole+1,
   };

   //Model implementation
   virtual bool          setData     ( const QModelIndex& index, const QVariant &value, int role   )       override;
   virtual QVariant      data        ( const QModelIndex& index, int role = Qt::DisplayRole        ) const override;
   virtual int           rowCount    ( const QModelIndex& parent = QModelIndex()                   ) const override;
   virtual Qt::ItemFlags flags       ( const QModelIndex& index                                    ) const override;
   virtual int           columnCount ( const QModelIndex& parent = QModelIndex()                   ) const override;
   virtual QModelIndex   parent      ( const QModelIndex& index                                    ) const override;
   virtual QModelIndex   index       ( int row, int column, const QModelIndex& parent=QModelIndex()) const override;
   virtual QVariant      headerData  ( int section, Qt::Orientation, int role = Qt::DisplayRole    ) const override;
   virtual QHash<int,QByteArray> roleNames() const override;

private:
   explicit ChainOfTrustModel(Certificate* c);
   virtual ~ChainOfTrustModel();

   ChainOfTrustModelPrivate* d_ptr;
   Q_DECLARE_PRIVATE(ChainOfTrustModel)
};

#endif
