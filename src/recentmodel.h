/************************************************************************************
 *   Copyright (C) 2015-2017 Savoir-faire Linux                                       *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>         *
 *            Alexandre Lision <alexandre.lision@savoirfairelinux.com>              *
 *                                                                                  *
 *   This library is free software; you can redistribute it and/or                  *
 *   modify it under the terms of the GNU Lesser General Public                     *
 *   License as published by the Free Software Foundation; either                   *
 *   version 2.1 of the License, or (at your option) any later version.             *
 *                                                                                  *
 *   This library is distributed in the hope that it will be useful,                *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of                 *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU              *
 *   Lesser General Public License for more details.                                *
 *                                                                                  *
 *   You should have received a copy of the GNU Lesser General Public               *
 *   License along with this library; if not, write to the Free Software            *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA *
 ***********************************************************************************/
#pragma once

#include <QtCore/QAbstractItemModel>
#include <QtCore/QItemSelectionModel>

#include <typedefs.h>

class QSortFilterProxyModel;
class RecentModelPrivate;
class Call;
class Person;
class ContactMethod;
class SmartListModel;

class LIB_EXPORT RecentModel : public QAbstractItemModel
{
   Q_OBJECT
public:

   //Model implementation
   virtual bool          setData     ( const QModelIndex& index, const QVariant& value, int role   ) override;
   virtual QVariant      data        ( const QModelIndex& index, int role = Qt::DisplayRole        ) const override;
   virtual int           rowCount    ( const QModelIndex& parent = QModelIndex()                   ) const override;
   virtual Qt::ItemFlags flags       ( const QModelIndex& index                                    ) const override;
   virtual int           columnCount ( const QModelIndex& parent = QModelIndex()                   ) const override;
   virtual QModelIndex   parent      ( const QModelIndex& index                                    ) const override;
   virtual QModelIndex   index       ( int row, int column, const QModelIndex& parent=QModelIndex()) const override;
   virtual QVariant      headerData  ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
   virtual QHash<int,QByteArray> roleNames() const override;

   //Proxy
   QSortFilterProxyModel* peopleProxy() const;

   //Singleton
   static RecentModel& instance();

   QModelIndex          getIndex     (Call* call               ) const;
   QModelIndex          getIndex     (Person* p                ) const;
   QModelIndex          getIndex     (ContactMethod* cm        ) const;
   bool                 isConference (const QModelIndex& idx   ) const;
   bool                 hasActiveCall(const QModelIndex& parent)      ;
   QVector<ContactMethod*> getContactMethods(const QModelIndex& idx) const;
   Call*                getActiveCall(const QModelIndex& parent)      ;
   QItemSelectionModel* selectionModel(                        ) const;
   QStringList          getParticipantName(Call* call          ) const;
   int                  getParticipantNumber(Call* call        ) const;
   SmartListModel*      getSmartListModel() const;

private:
   explicit RecentModel(QObject* parent = nullptr);
   virtual ~RecentModel();
   SmartListModel* smartListModel;

   RecentModelPrivate* d_ptr;
   Q_DECLARE_PRIVATE(RecentModel)
};
Q_DECLARE_METATYPE(RecentModel*)
