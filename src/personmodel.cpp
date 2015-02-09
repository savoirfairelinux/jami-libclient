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

//Parent
#include "personmodel.h"

//Ring library
#include "person.h"
#include "call.h"
#include "uri.h"
#include "contactmethod.h"
#include "collectioninterface.h"
#include "collectionmodel.h"
#include "collectioneditor.h"
#include "delegates/itemmodelstateserializationdelegate.h"

//Qt
#include <QtCore/QHash>
#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>

PersonModel* PersonModel::m_spInstance = nullptr;


class PersonModelPrivate : public QObject
{
   Q_OBJECT
public:
   PersonModelPrivate(PersonModel* parent);

   //Attributes
//    QVector<CollectionInterface*> m_lBackends;
   CommonCollectionModel* m_pBackendModel;
   QHash<QByteArray,PersonPlaceHolder*> m_hPlaceholders;

   //Indexes
   QHash<QByteArray,Person*> m_hPersonsByUid;
   QVector<Person*> m_lPersons;

private:
   PersonModel* q_ptr;

private Q_SLOTS:
   void slotReloaded();
//    void slotPersonAdded(Person* c);
};

PersonModelPrivate::PersonModelPrivate(PersonModel* parent) : QObject(parent), q_ptr(parent),
m_pBackendModel(nullptr)
{
   
}

///Constructor
PersonModel::PersonModel(QObject* par) : QAbstractItemModel(par?par:QCoreApplication::instance()), d_ptr(new PersonModelPrivate(this))
{
}

///Destructor
PersonModel::~PersonModel()
{
   d_ptr->m_hPersonsByUid.clear();
   while (d_ptr->m_lPersons.size()) {
      Person* c = d_ptr->m_lPersons[0];
      d_ptr->m_lPersons.remove(0);
      delete c;
   }
}

PersonModel* PersonModel::instance() {
   if (!m_spInstance)
      m_spInstance = new PersonModel(QCoreApplication::instance());
   return m_spInstance;
}

/*****************************************************************************
 *                                                                           *
 *                                   Model                                   *
 *                                                                           *
 ****************************************************************************/


bool PersonModel::setData( const QModelIndex& idx, const QVariant &value, int role)
{
   Q_UNUSED(idx)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}

QVariant PersonModel::data( const QModelIndex& idx, int role) const
{
   if (!idx.isValid())
      return QVariant();
   if (!idx.parent().isValid() && (role == Qt::DisplayRole || role == Qt::EditRole)) {
      const Person* c = d_ptr->m_lPersons[idx.row()];
      if (c)
         return QVariant(c->formattedName());
   }
   else if (idx.parent().isValid() && (role == Qt::DisplayRole || role == Qt::EditRole)) {
      const Person* c = d_ptr->m_lPersons[idx.parent().row()];
      if (c)
         return QVariant(c->phoneNumbers()[idx.row()]->uri());
   }
   return QVariant();
}

QVariant PersonModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   Q_UNUSED(section)
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
      return QVariant(tr("Persons"));
   return QVariant();
}

int PersonModel::rowCount( const QModelIndex& par ) const
{
   if (!par.isValid()) {
      return d_ptr->m_lPersons.size();
   }
   else if (!par.parent().isValid() && par.row() < d_ptr->m_lPersons.size()) {
      const Person* c = d_ptr->m_lPersons[par.row()];
      if (c) {
         const int size = c->phoneNumbers().size();
         return size==1?0:size;
      }
   }
   return 0;
}

Qt::ItemFlags PersonModel::flags( const QModelIndex& idx ) const
{
   if (!idx.isValid())
      return Qt::NoItemFlags;
   return Qt::ItemIsEnabled | ((idx.parent().isValid())?Qt::ItemIsSelectable:Qt::ItemIsEnabled);
}

int PersonModel::columnCount ( const QModelIndex& par) const
{
   Q_UNUSED(par)
   return 1;
}

QModelIndex PersonModel::parent( const QModelIndex& idx) const
{
   if (!idx.isValid())
      return QModelIndex();
   CategorizedCompositeNode* modelItem = (CategorizedCompositeNode*)idx.internalPointer();
   if (modelItem && modelItem->type() == CategorizedCompositeNode::Type::NUMBER) {
      int idx2 = d_ptr->m_lPersons.indexOf(((Person::ContactMethods*)modelItem)->contact());
      if (idx2 != -1) {
         return PersonModel::index(idx2,0,QModelIndex());
      }
   }
   return QModelIndex();
}

QModelIndex PersonModel::index( int row, int column, const QModelIndex& par) const
{
   if (!par.isValid() && d_ptr->m_lPersons.size() > row) {
      return createIndex(row,column,d_ptr->m_lPersons[row]);
   }
   else if (par.isValid() && d_ptr->m_lPersons[par.row()]->phoneNumbers().size() > row) {
      return createIndex(row,column,(CategorizedCompositeNode*)(&(d_ptr->m_lPersons[par.row()]->phoneNumbers())));
   }
   return QModelIndex();
}

/*****************************************************************************
 *                                                                           *
 *                                  Mutator                                  *
 *                                                                           *
 ****************************************************************************/


///Find contact by UID
Person* PersonModel::getPersonByUid(const QByteArray& uid)
{
   return d_ptr->m_hPersonsByUid[uid];
}

/**
 * Create a temporary contact or return the existing one for an UID
 * This temporary contact should eventually be merged into the real one
 */
Person* PersonModel::getPlaceHolder(const QByteArray& uid )
{
   Person* ct = d_ptr->m_hPersonsByUid[uid];

   //Do not create a placeholder if the real deal exist
   if (ct) {
      return ct;
   }

   //Do not re-create if it already exist
   ct = d_ptr->m_hPlaceholders[uid];
   if (ct)
      return ct;

   PersonPlaceHolder* ct2 = new PersonPlaceHolder(uid);

   d_ptr->m_hPlaceholders[ct2->uid()] = ct2;
   return ct2;
}

///Return if there is backends
// bool PersonModel::hasBackends() const
// {
//    return d_ptr->m_lBackends.size();
// }


// const QVector<CollectionInterface*> PersonModel::enabledBackends() const
// {
//    return d_ptr->m_lBackends;
// }

// bool PersonModel::hasEnabledBackends() const
// {
//    return d_ptr->m_lBackends.size()>0;
// }

// CommonCollectionModel* PersonModel::backendModel() const
// {
//    if (!d_ptr->m_pBackendModel) {
//       d_ptr->m_pBackendModel = new CommonCollectionModel(const_cast<PersonModel*>(this));
//    }
//    return d_ptr->m_pBackendModel; //TODO
// }

// QString PersonModel::backendCategoryName() const
// {
//    return tr("Persons");
// }

void PersonModel::backendAddedCallback(CollectionInterface* backend)
{
   Q_UNUSED(backend)
}

// const QVector<CollectionInterface*> PersonModel::backends() const
// {
//    return d_ptr->m_lBackends;
// }

bool PersonModel::addItemCallback(Person* item)
{
   addPerson(item);
   return true;
}

bool PersonModel::removeItemCallback(Person* item)
{
   Q_UNUSED(item)
   return false;
}

// bool PersonModel::enableBackend(CollectionInterface* backend, bool enabled)
// {
//    Q_UNUSED(backend)
//    Q_UNUSED(enabled)
//    //TODO;
//    return false;
// }

bool PersonModel::addPerson(Person* c)
{
   if (!c)
      return false;
   beginInsertRows(QModelIndex(),d_ptr->m_lPersons.size()-1,d_ptr->m_lPersons.size());
   d_ptr->m_lPersons << c;
   d_ptr->m_hPersonsByUid[c->uid()] = c;

   //Deprecate the placeholder
   if (d_ptr->m_hPlaceholders.contains(c->uid())) {
      PersonPlaceHolder* c2 = d_ptr->m_hPlaceholders[c->uid()];
      if (c2) {
         c2->merge(c);
         d_ptr->m_hPlaceholders[c->uid()] = nullptr;
      }
   }
   endInsertRows();
   emit layoutChanged();
   emit newPersonAdded(c);
   return true;
}


void PersonModel::disablePerson(Person* c)
{
   if (c)
      c->setActive(false);
}

const PersonList PersonModel::contacts() const
{
   return d_ptr->m_lPersons;
}

// void PersonModel::addBackend(CollectionInterface* backend, LoadOptions options)
// {
//    d_ptr->m_lBackends << backend;
//    connect(backend,SIGNAL(reloaded()),d_ptr.data(),SLOT(slotReloaded()));
//    connect(backend,SIGNAL(newPersonAdded(Person*)),d_ptr.data(),SLOT(slotPersonAdded(Person*)));
//    if (options & LoadOptions::FORCE_ENABLED || ItemModelStateSerializationDelegate::instance()->isChecked(backend))
//       backend->load();
//    emit newBackendAdded(backend);
// }

bool PersonModel::addNewPerson(Person* c, CollectionInterface* backend)
{
   Q_UNUSED(backend);
   return backends()[0]->editor<Person>()->addNew(c);
}


/*****************************************************************************
 *                                                                           *
 *                                    Slot                                   *
 *                                                                           *
 ****************************************************************************/

void PersonModelPrivate::slotReloaded()
{
   emit q_ptr->reloaded();
}


#include <personmodel.moc>
