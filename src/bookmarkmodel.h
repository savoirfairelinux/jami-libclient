/****************************************************************************
 *   Copyright (C) 2013-2015 by Savoir-Faire Linux                          *
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
#ifndef BOOKMARKMODEL_H
#define BOOKMARKMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QHash>
#include <QtCore/QStringList>
#include <QtCore/QDateTime>

//Ring
#include "collectionmanagerinterface.h"
#include "collectioninterface.h"
#include "typedefs.h"
#include "contactmethod.h"
// #include "person.h"
// #include "call.h"
class PersonBackend;
class NumberTreeBackend;

class BookmarkModelPrivate;
class CollectionInterface2;

class LIB_EXPORT BookmarkModel :  public QAbstractItemModel, public CollectionManagerInterface<ContactMethod>
{
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop
public:
   friend class NumberTreeBackend;
   //Constructor
   virtual ~BookmarkModel() {}
   explicit BookmarkModel(QObject* parent);

   //Setters
   void setRole(int role);
   void setShowAll(bool showAll);

   //Backend model implementation
   virtual bool clearAllBackends() const override;

   //Model implementation
   virtual bool          setData     ( const QModelIndex& index, const QVariant &value, int role   )       override;
   virtual bool          removeRows  ( int row, int count, const QModelIndex& parent=QModelIndex() )       override;
   virtual QVariant      data        ( const QModelIndex& index, int role = Qt::DisplayRole        ) const override;
   virtual int           rowCount    ( const QModelIndex& parent = QModelIndex()                   ) const override;
   virtual Qt::ItemFlags flags       ( const QModelIndex& index                                    ) const override;
   virtual int           columnCount ( const QModelIndex& parent = QModelIndex()                   ) const override;
   virtual QModelIndex   parent      ( const QModelIndex& index                                    ) const override;
   virtual QModelIndex   index       ( int row, int column, const QModelIndex& parent=QModelIndex()) const override;
   virtual QStringList   mimeTypes   (                                                             ) const override;
   virtual QMimeData*    mimeData    ( const QModelIndexList &indexes                              ) const override;
   virtual QVariant      headerData  ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
   virtual QHash<int,QByteArray> roleNames() const override;

   //Management
   void remove        (const QModelIndex& idx );
   void addBookmark   (ContactMethod* number    );
   void removeBookmark(ContactMethod* number    );

   //Getters
   int          acceptedPayloadTypes();
   ContactMethod* getNumber(const QModelIndex& idx);

   //Singleton
   static BookmarkModel* instance();

private:
   static BookmarkModel* m_spInstance;

   BookmarkModelPrivate* d_ptr;
   Q_DECLARE_PRIVATE(BookmarkModel)

   //Backend interface
   virtual void backendAddedCallback(CollectionInterface* backend) override;
   virtual bool addItemCallback(ContactMethod* item) override;
   virtual bool removeItemCallback(ContactMethod* item) override;

public Q_SLOTS:
   void reloadCategories();
};

#endif
