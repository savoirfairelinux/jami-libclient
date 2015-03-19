/****************************************************************************
 *   Copyright (C) 2014-2015 by Savoir-Faire Linux                          *
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
#ifndef CONTACTMODEL_H
#define CONTACTMODEL_H

#include <QObject>
#include <QHash>
#include <QStringList>
#include <QVariant>
#include <QtCore/QAbstractItemModel>

#include "typedefs.h"
#include "person.h"
#include "collectionmanagerinterface.h"

//Ring
class Person;
class Account;
class CollectionInterface;
class PersonModelPrivate;
class PersonItemNode;

//Typedef
typedef QVector<Person*> PersonList;

///PersonModel: Allow different way to handle contact without poluting the library
class LIB_EXPORT PersonModel :
   public QAbstractItemModel, public CollectionManagerInterface<Person> {
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop
   friend class PersonItemNode;
public:

   template <typename T > using ItemMediator = CollectionMediator<Person>;


   explicit PersonModel(QObject* parent = nullptr);
   virtual ~PersonModel();

   //Mutator
   bool addPerson(Person* c);
   void disablePerson(Person* c);

   //Getters
   Person* getPersonByUid   ( const QByteArray& uid );
   Person* getPlaceHolder(const QByteArray& uid );

   //Model implementation
   virtual bool          setData     ( const QModelIndex& index, const QVariant &value, int role   ) override;
   virtual QVariant      data        ( const QModelIndex& index, int role = Qt::DisplayRole        ) const override;
   virtual int           rowCount    ( const QModelIndex& parent = QModelIndex()                   ) const override;
   virtual Qt::ItemFlags flags       ( const QModelIndex& index                                    ) const override;
   virtual int           columnCount ( const QModelIndex& parent = QModelIndex()                   ) const override;
   virtual QModelIndex   parent      ( const QModelIndex& index                                    ) const override;
   virtual QModelIndex   index       ( int row, int column, const QModelIndex& parent=QModelIndex()) const override;
   virtual QVariant      headerData  ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
   virtual QHash<int,QByteArray> roleNames() const override;

   //Singleton
   static PersonModel* instance();

private:
   QScopedPointer<PersonModelPrivate> d_ptr;
   Q_DECLARE_PRIVATE(PersonModel)

   //Singleton
   static PersonModel* m_spInstance;

   //Backend interface
   virtual void collectionAddedCallback(CollectionInterface* backend) override;
   virtual bool addItemCallback(const Person* item) override;
   virtual bool removeItemCallback(const Person* item) override;

public Q_SLOTS:
   bool addNewPerson(Person* c, CollectionInterface* backend = nullptr);

Q_SIGNALS:
   void newPersonAdded(const Person* c);
   void newBackendAdded(CollectionInterface* backend);
};


#endif
