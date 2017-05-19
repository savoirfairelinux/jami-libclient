/****************************************************************************
 *   Copyright (C) 2014-2016 by Savoir-faire Linux                          *
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

//Std
#include <memory>
#include <vector>

//Ring library
#include "person.h"
#include "call.h"
#include "uri.h"
#include "contactmethod.h"
#include "collectioninterface.h"
#include "collectionmodel.h"
#include "collectioneditor.h"
#include "transitionalpersonbackend.h"

//Qt
#include <QtCore/QHash>
#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>

class PersonItemNode
{
public:

   enum class NodeType {
      PERSON,
      NUMBER,
   };

   PersonItemNode(Person* p, const NodeType type);
   PersonItemNode(ContactMethod* cm, const NodeType type);
   std::unique_ptr<Person> m_pPerson;
   ContactMethod* m_pContactMethod {nullptr};
   int m_Index;
   std::vector<std::unique_ptr<PersonItemNode>> m_lChildren;
   PersonItemNode* m_pParent {nullptr};
   NodeType m_Type;

};

class PersonModelPrivate final : public QObject
{
   Q_OBJECT
public:
   PersonModelPrivate(PersonModel* parent);

   //Attributes
//    QVector<CollectionInterface*> m_lBackends;
   QHash<QByteArray,PersonPlaceHolder*> m_hPlaceholders;

   //Indexes
   QHash<QByteArray,Person*> m_hPersonsByUid;
   std::vector<std::unique_ptr<PersonItemNode>> m_lPersons;

private:
   PersonModel* q_ptr;
//    void slotPersonAdded(Person* c);

public Q_SLOTS:
   void slotLastUsedTimeChanged(time_t t) const;
};

PersonItemNode::PersonItemNode(Person* p, const NodeType type) :
m_Type(type),m_pPerson(p)
{

}

PersonItemNode::PersonItemNode(ContactMethod* cm, const NodeType type) :
m_Type(type),m_pContactMethod(cm)
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
}

PersonModel& PersonModel::instance()
{
    static auto instance = new PersonModel(QCoreApplication::instance());
    return *instance;
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
   else if (!par.parent().isValid() && static_cast<unsigned>(par.row()) < d_ptr->m_lPersons.size()) {
      return d_ptr->m_lPersons[par.row()]->m_lChildren.size();
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
   if (row >= 0 && column >= 0) {
      if (!par.isValid() && d_ptr->m_lPersons.size() > static_cast<unsigned>(row)) {
         return createIndex(row,column,d_ptr->m_lPersons[row].get());
      } else if (par.isValid() && d_ptr->m_lPersons[par.row()]->m_lChildren.size() > static_cast<unsigned>(row)) {
         PersonItemNode* modelItem = (PersonItemNode*)par.internalPointer();
         if (modelItem && static_cast<unsigned>(row) < modelItem->m_lChildren.size())
            return createIndex(row,column,modelItem->m_lChildren[row].get());
      }
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
   d_ptr->m_lPersons.emplace_back(new PersonItemNode {const_cast<Person*>(c), PersonItemNode::NodeType::PERSON});
   auto& inode = *d_ptr->m_lPersons.back();
   inode.m_Index = d_ptr->m_lPersons.size() - 1;
   d_ptr->m_hPersonsByUid[c->uid()] = const_cast<Person*>(c);
   endInsertRows();
   emit newPersonAdded(c);

   //Add the contact method nodes
   const QModelIndex& idx = index(inode.m_Index,0);
   beginInsertRows(idx,0,c->phoneNumbers().size());
   inode.m_lChildren.reserve(c->phoneNumbers().size());
   for (auto& m : c->phoneNumbers()) {
      inode.m_lChildren.emplace_back(new PersonItemNode {m, PersonItemNode::NodeType::NUMBER});
      auto& child = *inode.m_lChildren.back().get();
      child.m_Index = inode.m_lChildren.size() - 1;
      child.m_pParent = &inode; //TODO support adding new contact methods on the fly
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

   connect(c, &Person::lastUsedTimeChanged, d_ptr.data(), &PersonModelPrivate::slotLastUsedTimeChanged);

   if (c->lastUsedTime())
      emit lastUsedTimeChanged(const_cast<Person*>(c), c->lastUsedTime());

   return true;
}

bool PersonModel::removeItemCallback(const Person* item)
{
   for (unsigned int nodeIdx = 0; nodeIdx < d_ptr->m_lPersons.size(); ++nodeIdx) {
      auto person = d_ptr->m_lPersons[nodeIdx]->m_pPerson.get();
      if (person == item) {

          for ( const auto cm : person->phoneNumbers() )
              // cm is not linked to any person anymore
              cm->setPerson(nullptr);

          // Remove contact
          beginRemoveRows(QModelIndex(), nodeIdx, nodeIdx);
          d_ptr->m_lPersons[nodeIdx].release();
          d_ptr->m_lPersons.erase(d_ptr->m_lPersons.begin() + nodeIdx);

          // update indexes
          for (unsigned int i = 0; i < d_ptr->m_lPersons.size(); ++i) {
             d_ptr->m_lPersons[i]->m_Index = i;
             for (unsigned int j = 0; j < d_ptr->m_lPersons[i]->m_lChildren.size(); ++j)
                d_ptr->m_lPersons[i]->m_lChildren[j]->m_Index = j;
          }
          endRemoveRows();

          //Deprecate the placeholder
          if (d_ptr->m_hPlaceholders.contains(item->uid())) {
             PersonPlaceHolder* placeholder = d_ptr->m_hPlaceholders[item->uid()];
             if (placeholder)
                d_ptr->m_hPlaceholders[item->uid()] = nullptr;
          }
          break;
      }
   }

   emit personRemoved(item);
   return item;
}

///When we get a peer profile, its a vCard from a ContactRequest or a Call. We need to verify if
///this is Person which already exists, and so we simply need to update our existing vCard, or if
///this is a new Person, in which case we'll save a new vCard.
///We cannot trust the UID in the vCard for uniqueness. We can only rely on the RingID to be unique.
bool PersonModel::addPeerProfile(Person* person)
{
   if (!person or not person->collection()) return false;

   // check if this person is saved in the PeerProfileCollection, "ppc"
   if (person->collection() != &TransitionalPersonBackend::instance() and
       person->collection()->name() != "ppc")
   {
      qWarning() << "About to add Person to the PeerProfileCollection which is part of another collection";
   }

   for (auto col : collections(CollectionInterface::SupportedFeatures::ADD)) {
       //Only add profile to peer profile collection
       if (col->id() == "ppc") {
           col->add(person);
           return true;
       }
   }
   return false;
}

///@deprecated
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

void PersonModelPrivate::slotLastUsedTimeChanged(time_t t) const
{
   emit q_ptr->lastUsedTimeChanged(static_cast<Person*>(QObject::sender()), t);
}


#include <personmodel.moc>
