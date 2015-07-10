/****************************************************************************
 *   Copyright (C) 2013-2014 by Savoir-Faire Linux                          *
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
#ifndef PROFILEMODEL_H
#define PROFILEMODEL_H

#include "typedefs.h"
#include "person.h"
#include "account.h"
#include <QStringList>
#include <QSharedPointer>
#include <QtCore/QAbstractItemModel>

class ProfileContentBackend;
class ProfilePersisterDelegate;
class VCardMapper;
class ProfileModelPrivate;
class QItemSelectionModel;

template<typename T> class CollectionMediator;

class LIB_EXPORT ProfileModel : public QAbstractItemModel {
   Q_OBJECT
   friend class ProfileContentBackend;
public:
   explicit ProfileModel(QObject* parent = nullptr);
   virtual ~ProfileModel();
   static ProfileModel* instance();

   //Abstract model member
   virtual QVariant      data        ( const QModelIndex& index, int role = Qt::DisplayRole         ) const override;
   virtual int           rowCount    ( const QModelIndex& parent = QModelIndex()                    ) const override;
   virtual int           columnCount ( const QModelIndex& parent = QModelIndex()                    ) const override;
   virtual Qt::ItemFlags flags       ( const QModelIndex& index                                     ) const override;
   virtual bool          setData     ( const QModelIndex& index, const QVariant &value, int role    )       override;
   virtual QModelIndex   index       ( int row, int column, const QModelIndex& parent=QModelIndex() ) const override;
   virtual QModelIndex   parent      ( const QModelIndex& index                                     ) const override;
   virtual QVariant      headerData  ( int section, Qt::Orientation orientation, int role           ) const override;
   virtual QStringList   mimeTypes   (                                                              ) const override;
   virtual QMimeData*    mimeData    ( const QModelIndexList &indexes                               ) const override;
   virtual bool          dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
   virtual QHash<int,QByteArray> roleNames() const override;

   //Getter
   QModelIndex mapToSource  (const QModelIndex& idx) const;
   QModelIndex mapFromSource(const QModelIndex& idx) const;
   int acceptedPayloadTypes() const;
   QItemSelectionModel* selectionModel() const;

private:
   ProfileModelPrivate* d_ptr;
   Q_DECLARE_PRIVATE(ProfileModel)

   //Singleton
   static ProfileModel* m_spInstance;

public Q_SLOTS:
//    bool addNewProfile(Person* c, CollectionInterface* backend = nullptr);
//    bool createProfile(const QString& name);
};

#endif
