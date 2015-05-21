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


class PersonItemNode
{
public:

   enum class NodeType {
      PERSON,
      NUMBER,
   };

   PersonItemNode(Person* p, const NodeType type);
   PersonItemNode(ContactMethod* cm, const NodeType type);
   Person* m_pPerson;
   ContactMethod* m_pContactMethod;
   int m_Index;
   QVector<PersonItemNode*> m_lChildren;
   PersonItemNode* m_pParent;
   NodeType m_Type;

};

class PersonModelPrivate : public QObject
{
   Q_OBJECT
public:
   PersonModelPrivate(PersonModel* parent);

   //Attributes
//    QVector<CollectionInterface*> m_lBackends;
   QHash<QByteArray,PersonPlaceHolder*> m_hPlaceholders;

   //Indexes
   QHash<QByteArray,Person*> m_hPersonsByUid;
   QVector<PersonItemNode*> m_lPersons;

private:
   PersonModel* q_ptr;
//    void slotPersonAdded(Person* c);
};

PersonItemNode::PersonItemNode(Person* p, const NodeType type) :
m_Type(type),m_pPerson(p),m_pContactMethod(nullptr),m_pParent(nullptr)
{

}

PersonItemNode::PersonItemNode(ContactMethod* cm, const NodeType type) :
m_Type(type),m_pContactMethod(cm),m_pPerson(nullptr),m_pParent(nullptr)
{

}

PersonModelPrivate::PersonModelPrivate(PersonModel* parent) : QObject(parent), q_ptr(parent)
{

}

///Constructor
PersonModel::PersonModel(QObject* par) : QAbstractItemModel(par?par:QCoreApplication::instance()), CollectionManagerInterface<Person>(this),
d_ptr(new PersonModelPrivate(this))
{
   setObjectName("PersonModel");
}

///Destructor
PersonModel::~PersonModel()
{
   d_ptr->m_hPersonsByUid.clear();
   while (d_ptr->m_lPersons.size()) {
      PersonItemNode* cn = d_ptr->m_lPersons[0];
      d_ptr->m_lPersons.remove(0);
      if (cn->m_pPerson)
         delete cn->m_pPerson;
      for(PersonItemNode* n : cn->m_lChildren)
         delete n;
      delete cn;
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

QHash<int,QByteArray> PersonModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
      roles[ (int)Person::Role::Organization      ] = "organization";
      roles[ (int)Person::Role::Group             ] = "group";
      roles[ (int)Person::Role::Department        ] = "department";
      roles[ (int)Person::Role::PreferredEmail    ] = "preferredEmail";
      roles[ (int)Person::Role::FormattedLastUsed ] = "formattedLastUsed";
      roles[ (int)Person::Role::IndexedLastUsed   ] = "indexedLastUsed";
      roles[ (int)Person::Role::DatedLastUsed     ] = "datedLastUsed";
      roles[ (int)Person::Role::Filter            ] = "filter"; //All roles, all at once
      roles[ (int)Person::Role::DropState         ] = "dropState"; //State for drag and drop
   }
   return roles;
}

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
   const PersonItemNode* c = static_cast<PersonItemNode*>(idx.internalPointer());

   switch(c->m_Type) {
      case PersonItemNode::NodeType::PERSON:
         return c->m_pPerson->roleData(role);
      case PersonItemNode::NodeType::NUMBER:
         return c->m_pContactMethod->roleData(role);
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
      const PersonItemNode* c = d_ptr->m_lPersons[par.row()];
      if (c) {
         return c->m_lChildren.size();
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
   PersonItemNode* modelItem = (PersonItemNode*)idx.internalPointer();
   if (modelItem && modelItem->m_pParent) {
      return createIndex(modelItem->m_pParent->m_Index,0,modelItem->m_pParent);
   }
   return QModelIndex();
}

QModelIndex PersonModel::index( int row, int column, const QModelIndex& par) const
{
   if (!par.isValid() && d_ptr->m_lPersons.size() > row) {
      return createIndex(row,column,d_ptr->m_lPersons[row]);
   }
   else if (par.isValid() && d_ptr->m_lPersons[par.row()]->m_lChildren.size() > row) {
      PersonItemNode* modelItem = (PersonItemNode*)par.internalPointer();
      if (modelItem && row < modelItem->m_lChildren.size())
         return createIndex(row,column,modelItem->m_lChildren[row]);
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

void PersonModel::collectionAddedCallback(CollectionInterface* backend)
{
   Q_UNUSED(backend)
}

bool PersonModel::addItemCallback(const Person* c)
{
   //Add to the model
   beginInsertRows(QModelIndex(),d_ptr->m_lPersons.size(),d_ptr->m_lPersons.size());
   PersonItemNode* n = new PersonItemNode(const_cast<Person*>(c),PersonItemNode::NodeType::PERSON);
   n->m_Index = d_ptr->m_lPersons.size();
   d_ptr->m_lPersons << n;
   d_ptr->m_hPersonsByUid[c->uid()] = const_cast<Person*>(c);
   endInsertRows();
   emit newPersonAdded(c);

   //Add the contact method nodes
   const QModelIndex& idx = index(n->m_Index,0);
   beginInsertRows(idx,0,c->phoneNumbers().size());
   for(ContactMethod* m : c->phoneNumbers() ) {
      PersonItemNode* n2 = new PersonItemNode(m,PersonItemNode::NodeType::NUMBER);
      n2->m_Index = n->m_lChildren.size();
      n2->m_pParent = n; //TODO support adding new contact methods on the fly
      n->m_lChildren << n2;
   }
   endInsertRows();

   //Deprecate the placeholder
   if (d_ptr->m_hPlaceholders.contains(c->uid())) {
      PersonPlaceHolder* c2 = d_ptr->m_hPlaceholders[c->uid()];
      if (c2) {
         c2->merge(const_cast<Person*>(c));
         d_ptr->m_hPlaceholders[c->uid()] = nullptr;
      }
   }

   return true;
}

bool PersonModel::removeItemCallback(const Person* item)
{
   if (item)
      emit const_cast<Person*>(item)->changed();
   return item;
}

bool PersonModel::addPerson(Person* c)
{
   if (!c)
      return false;
   if (collections().size()) //TODO this is wrong, it work for now because profilemodel is [0]
      collections()[0]->add(c);
   return true;
}

bool PersonModel::addNewPerson(Person* c, CollectionInterface* backend)
{
   if ((!backend) && (!collections().size()))
      return false;

   bool ret = false;

   if (backend) {
      ret |= backend->editor<Person>()->addNew(c);
   }
   else for (CollectionInterface* col :collections(CollectionInterface::SupportedFeatures::ADD)) {
      if (col->id() != "trcb") //Do not add to the transitional contact backend
         ret |= col->editor<Person>()->addNew(c);
   }

   return ret;
}


#include <personmodel.moc>
