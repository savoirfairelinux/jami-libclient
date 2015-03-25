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
#include "categorizedcontactmodel.h"

//Qt
#include <QtCore/QDebug>
#include <QtCore/QDate>
#include <QtCore/QMimeData>
#include <QtCore/QCoreApplication>

//Ring
#include "callmodel.h"
#include "categorizedhistorymodel.h"
#include "contactmethod.h"
#include "phonedirectorymodel.h"
#include "historytimecategorymodel.h"
#include "person.h"
#include "uri.h"
#include "mime.h"
#include "personmodel.h"

class ContactTreeNode;

class ContactTreeNode : public CategorizedCompositeNode {
public:
   friend class CategorizedContactModel;
   friend class CategorizedContactModelPrivate;
   friend class ContactTreeBinder;

   enum class NodeType {
      PERSON       ,
      CONTACTMETHOD,
      CATEGORY     ,
   };

   //Constructor
   ContactTreeNode( const Person* ct    , CategorizedContactModel* parent);
   ContactTreeNode( ContactMethod* cm   , CategorizedContactModel* parent);
   ContactTreeNode( const QString& name , CategorizedContactModel* parent);
   virtual ~ContactTreeNode();

   virtual QObject* getSelf() const override;

   //Attributes
   const Person*             m_pContact      ;
   ContactMethod*            m_pContactMethod;
   uint                      m_Index         ;
   QString                   m_Name          ;
   NodeType                  m_Type          ;
   ContactTreeNode*          m_pParent       ;
   QVector<ContactTreeNode*> m_lChildren     ;
   CategorizedContactModel*  m_pModel        ;

   //Helpers
   void slotChanged                        (       );
   void slotContactMethodCountChanged      (int,int);
   void slotContactMethodCountAboutToChange(int,int);
};

class CategorizedContactModelPrivate : public QObject
{
   Q_OBJECT
public:
   CategorizedContactModelPrivate(CategorizedContactModel* parent);

   //Helpers
   QString category(const Person* ct) const;

   //Attributes
   QHash<Person*, time_t>          m_hContactByDate   ;
   QVector<ContactTreeNode*>       m_lCategoryCounter ;
   QHash<QString,ContactTreeNode*> m_hCategories      ;
   int                             m_Role             ;
   QStringList                     m_lMimes           ;
   bool                            m_SortAlphabetical ;
   QString                         m_DefaultCategory  ;

   //Helper
   ContactTreeNode* getContactTopLevelItem(const QString& category);
   QModelIndex getIndex(int row, int column, ContactTreeNode* parent);

private:
   CategorizedContactModel* q_ptr;

public Q_SLOTS:
   void reloadCategories();
   void slotContactAdded(const Person* c);
};

ContactTreeNode::ContactTreeNode(const Person* ct, CategorizedContactModel* parent) : CategorizedCompositeNode(CategorizedCompositeNode::Type::CONTACT),
   m_pContact(ct),m_Index(-1),m_pContactMethod(nullptr),m_Type(ContactTreeNode::NodeType::PERSON),m_pParent(nullptr),m_pModel(parent)
{
   QObject::connect(m_pContact,&Person::changed                      ,[this](            ){ slotChanged                        (   ); });
   QObject::connect(m_pContact,&Person::phoneNumberCountChanged      ,[this](int n, int o){ slotContactMethodCountChanged      (n,o); });
   QObject::connect(m_pContact,&Person::phoneNumberCountAboutToChange,[this](int n, int o){ slotContactMethodCountAboutToChange(n,o); });
}

ContactTreeNode::ContactTreeNode(ContactMethod* cm, CategorizedContactModel* parent) : CategorizedCompositeNode(CategorizedCompositeNode::Type::NUMBER),
   m_pContactMethod(cm),m_Index(-1),m_pContact(nullptr),m_Type(ContactTreeNode::NodeType::CONTACTMETHOD),m_pParent(nullptr),m_pModel(parent)
{
   QObject::connect(m_pContactMethod,&ContactMethod::changed,[this](){ slotChanged(); });
}

ContactTreeNode::ContactTreeNode(const QString& name, CategorizedContactModel* parent) : CategorizedCompositeNode(CategorizedCompositeNode::Type::CONTACT),
   m_pContactMethod(nullptr),m_Index(-1),m_pContact(nullptr),m_Type(ContactTreeNode::NodeType::CATEGORY),m_Name(name),m_pParent(nullptr),m_pModel(parent)
{
}

ContactTreeNode::~ContactTreeNode()
{
}

QModelIndex CategorizedContactModelPrivate::getIndex(int row, int column, ContactTreeNode* parent)
{
   return q_ptr->createIndex(row,column,parent);
}

QObject* ContactTreeNode::getSelf() const
{
   return (QObject*)m_pContact;
}

void ContactTreeNode::slotChanged()
{
   const QModelIndex& self = m_pModel->d_ptr->getIndex(m_Index,0,this);
   emit m_pModel->dataChanged(self,self);
}

void ContactTreeNode::slotContactMethodCountChanged(int count, int oldCount)
{
   const QModelIndex idx = m_pModel->d_ptr->getIndex(m_Index,0,this);
   if (count > oldCount) {

      //After discussion, it was decided that contacts with only 1 phone number should
      //be handled differently and the additional complexity isn't worth it
      if (oldCount == 1)
         oldCount = 0;

      m_pModel->beginInsertRows(idx,oldCount,count-1);
      for (int i = count; i < oldCount; i++) {
         ContactTreeNode* n2 = new ContactTreeNode(m_pContact->phoneNumbers()[i],m_pModel);
         n2->m_Index = m_lChildren.size();
         n2->m_pParent = this;
         m_lChildren << n2;
      }
      m_pModel->endInsertRows();
   }
   emit m_pModel->dataChanged(idx,idx);
}

void ContactTreeNode::slotContactMethodCountAboutToChange(int count, int oldCount)
{
   const QModelIndex idx = m_pModel->d_ptr->getIndex(m_Index,0,this);
   if (count < oldCount) {
      //If count == 1, disable all children
      m_pModel->beginRemoveRows(idx,count == 1?0:count,oldCount-1);
      //FIXME memory leak
      m_pModel->endRemoveRows();
   }
}

CategorizedContactModelPrivate::CategorizedContactModelPrivate(CategorizedContactModel* parent) : QObject(parent), q_ptr(parent),
m_lCategoryCounter(),m_Role(Qt::DisplayRole),m_SortAlphabetical(true)
{

}

//
CategorizedContactModel::CategorizedContactModel(int role) : QAbstractItemModel(QCoreApplication::instance()),d_ptr(new CategorizedContactModelPrivate(this))
{
   setObjectName("CategorizedContactModel");
   d_ptr->m_Role    = role;
   d_ptr->m_lCategoryCounter.reserve(32);
   d_ptr->m_lMimes << RingMimes::PLAIN_TEXT << RingMimes::PHONENUMBER;

   connect(PersonModel::instance(),&PersonModel::newPersonAdded,d_ptr.data(),&CategorizedContactModelPrivate::slotContactAdded);

   for(int i=0; i < PersonModel::instance()->rowCount();i++) {
      Person* p = qvariant_cast<Person*>(PersonModel::instance()->index(i,0).data((int)Person::Role::Object));
      d_ptr->slotContactAdded(p);
   }

}

CategorizedContactModel::~CategorizedContactModel()
{
   foreach(ContactTreeNode* item,d_ptr->m_lCategoryCounter) {
      delete item;
   }
}

QHash<int,QByteArray> CategorizedContactModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
      roles.insert((int)Person::Role::Organization      ,QByteArray("organization")     );
      roles.insert((int)Person::Role::Group             ,QByteArray("group")            );
      roles.insert((int)Person::Role::Department        ,QByteArray("department")       );
      roles.insert((int)Person::Role::PreferredEmail    ,QByteArray("preferredEmail")   );
      roles.insert((int)Person::Role::FormattedLastUsed ,QByteArray("formattedLastUsed"));
      roles.insert((int)Person::Role::IndexedLastUsed   ,QByteArray("indexedLastUsed")  );
      roles.insert((int)Person::Role::DatedLastUsed     ,QByteArray("datedLastUsed")    );
      roles.insert((int)Person::Role::Filter            ,QByteArray("filter")           );
      roles.insert((int)Person::Role::DropState         ,QByteArray("dropState")        );
   }
   return roles;
}

ContactTreeNode* CategorizedContactModelPrivate::getContactTopLevelItem(const QString& category)
{
   if (!m_hCategories[category]) {
      ContactTreeNode* item = new ContactTreeNode(category,q_ptr);
      m_hCategories[category] = item;
      item->m_Index = m_lCategoryCounter.size();
//       emit layoutAboutToBeChanged();
      q_ptr->beginInsertRows(QModelIndex(),m_lCategoryCounter.size(),m_lCategoryCounter.size()); {
         m_lCategoryCounter << item;
      } q_ptr->endInsertRows();
//       emit layoutChanged();
   }
   ContactTreeNode* item = m_hCategories[category];
   return item;
}

void CategorizedContactModelPrivate::reloadCategories()
{
   emit q_ptr->layoutAboutToBeChanged(); //FIXME far from optimal
   q_ptr->beginResetModel();
   m_hCategories.clear();
   q_ptr->beginRemoveRows(QModelIndex(),0,m_lCategoryCounter.size()-1);
   foreach(ContactTreeNode* item,m_lCategoryCounter) {
      delete item;
   }
   q_ptr->endRemoveRows();
   m_lCategoryCounter.clear();
   for(int i=0; i < PersonModel::instance()->rowCount();i++) {
      Person* cont = qvariant_cast<Person*>(PersonModel::instance()->index(i,0).data((int)Person::Role::Object));
      slotContactAdded(cont);
   }
   q_ptr->endResetModel();
   emit q_ptr->layoutChanged();
}

void CategorizedContactModelPrivate::slotContactAdded(const Person* c)
{
   if (!c) return;

   const QString val = category(c);
   ContactTreeNode* item = getContactTopLevelItem(val);
   ContactTreeNode* contactNode = new ContactTreeNode(c,q_ptr);
   contactNode->m_pParent = item;
   contactNode->m_Index = item->m_lChildren.size();
   //emit layoutAboutToBeChanged();
   q_ptr->beginInsertRows(q_ptr->index(item->m_Index,0,QModelIndex()),item->m_lChildren.size(),item->m_lChildren.size()); {
      item->m_lChildren << contactNode;
   } q_ptr->endInsertRows();

   q_ptr->beginInsertRows(q_ptr->createIndex(contactNode->m_Index,0,contactNode),0,c->phoneNumbers().size());
   if (c->phoneNumbers().size() > 1) {
      for (ContactMethod* m : c->phoneNumbers() ) {
         ContactTreeNode* n2 = new ContactTreeNode(m,q_ptr);
         n2->m_Index = contactNode->m_lChildren.size();
         n2->m_pParent = contactNode;
         contactNode->m_lChildren << n2;
      }
   }
   q_ptr->endInsertRows();

   //emit layoutChanged();
}

bool CategorizedContactModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   if (index.isValid() && index.parent().isValid()) {
      ContactTreeNode* modelItem = (ContactTreeNode*)index.internalPointer();
      if (modelItem && role == (int)Person::Role::DropState && modelItem->m_Type == ContactTreeNode::NodeType::PERSON) {
         modelItem->setDropState(value.toInt());
         emit dataChanged(index, index);
         return true;
      }
   }
   return false;
}

QVariant CategorizedContactModel::data( const QModelIndex& index, int role) const
{
   if (!index.isValid())
      return QVariant();

   ContactTreeNode* modelItem = (ContactTreeNode*)index.internalPointer();
   switch (modelItem->m_Type) {
      case ContactTreeNode::NodeType::CATEGORY:
      switch (role) {
         case Qt::DisplayRole:
            return static_cast<const ContactTreeNode*>(modelItem)->m_Name;
         case (int)Person::Role::IndexedLastUsed:
            return index.child(0,0).data((int)Person::Role::IndexedLastUsed);
         default:
            break;
      }
      break;
   case ContactTreeNode::NodeType::PERSON:{
      const Person* c = static_cast<Person*>(modelItem->getSelf());
      switch (role) {
         case (int)Person::Role::DropState:
            return QVariant(modelItem->dropState());
         default:
            break;
      }
      return c->roleData(role);
      break;
   }
   case ContactTreeNode::NodeType::CONTACTMETHOD: /* && (role == Qt::DisplayRole)) {*/
      return modelItem->m_pContactMethod->roleData(role);
   }
   return QVariant();
}

QVariant CategorizedContactModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   Q_UNUSED(section)
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
      return QVariant(tr("Contacts"));
   return QVariant();
}

bool CategorizedContactModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
   Q_UNUSED( action )
   setData(parent,-1,static_cast<int>(Call::Role::DropState));
   if (data->hasFormat(RingMimes::CALLID)) {
      const QByteArray encodedCallId = data->data( RingMimes::CALLID    );
      const QModelIndex targetIdx    = index   ( row,column,parent );
      Call* call                     = CallModel::instance()->fromMime ( encodedCallId        );
      if (call && targetIdx.isValid()) {
         ContactTreeNode* modelItem = (ContactTreeNode*)targetIdx.internalPointer();
         switch (modelItem->m_Type) {
            case ContactTreeNode::NodeType::PERSON: {
               const Person* ct = static_cast<Person*>(modelItem->getSelf());
               if (ct) {
                  switch(ct->phoneNumbers().size()) {
                     case 0: //Do nothing when there is no phone numbers
                        return false;
                     case 1: //Call when there is one
                        CallModel::instance()->transfer(call,ct->phoneNumbers()[0]);
                        break;
                     default:
                        //TODO
                        break;
                  };
               }
            } break;
            case ContactTreeNode::NodeType::CONTACTMETHOD: {
               const ContactMethod* nb  = modelItem->m_pContactMethod;
               if (nb) {
                  call->setTransferNumber(nb->uri());
                  CallModel::instance()->transfer(call,nb);
               }
            } break;
            case ContactTreeNode::NodeType::CATEGORY:
               break;
         }
      }
   }
   return false;
}


int CategorizedContactModel::rowCount( const QModelIndex& parent ) const
{
   if (!parent.isValid() || !parent.internalPointer())
      return d_ptr->m_lCategoryCounter.size();
   const ContactTreeNode* parentNode = static_cast<ContactTreeNode*>(parent.internalPointer());

   if (parentNode)
      return parentNode->m_lChildren.size();

   return 0;
}

Qt::ItemFlags CategorizedContactModel::flags( const QModelIndex& index ) const
{
   if (!index.isValid())
      return Qt::NoItemFlags;

   const ContactTreeNode* modelNode = static_cast<ContactTreeNode*>(index.internalPointer());

   return (((!modelNode->m_pContact) || (modelNode->m_pContact->isActive())) ? Qt::ItemIsEnabled : Qt::NoItemFlags )
      | Qt::ItemIsSelectable
      | (modelNode->m_pParent? (Qt::ItemIsDragEnabled|Qt::ItemIsDropEnabled) : Qt::ItemIsEnabled
   );
}

int CategorizedContactModel::columnCount ( const QModelIndex& parent) const
{
   Q_UNUSED(parent)
   return 1;
}

QModelIndex CategorizedContactModel::parent( const QModelIndex& index) const
{
   if (!index.isValid() || !index.internalPointer())
      return QModelIndex();

   const ContactTreeNode* modelItem = static_cast<ContactTreeNode*>(index.internalPointer());

   if (modelItem && modelItem->m_pParent)
      return createIndex(modelItem->m_pParent->m_Index,0,modelItem->m_pParent);

   return QModelIndex();
}

QModelIndex CategorizedContactModel::index( int row, int column, const QModelIndex& parent) const
{
   if (!parent.isValid() && row < d_ptr->m_lCategoryCounter.size()) {
      return createIndex(row,column,d_ptr->m_lCategoryCounter[row]);
   }

   ContactTreeNode* parentNode = static_cast<ContactTreeNode*>(parent.internalPointer());

   if (parentNode && row < parentNode->m_lChildren.size())
      return createIndex(row,column,parentNode->m_lChildren[row]);

   return QModelIndex();
}

QStringList CategorizedContactModel::mimeTypes() const
{
   return d_ptr->m_lMimes;
}

QMimeData* CategorizedContactModel::mimeData(const QModelIndexList &indexes) const
{
   QMimeData *mimeData = new QMimeData();
   foreach (const QModelIndex &index, indexes) {
      if (index.isValid()) {
         const ContactTreeNode* modelItem = static_cast<ContactTreeNode*>(index.internalPointer());
         switch(modelItem->m_Type) {
            case ContactTreeNode::NodeType::PERSON: {
               //Contact
               const Person* ct = static_cast<Person*>(modelItem->getSelf());
               if (ct) {
                  if (ct->phoneNumbers().size() == 1) {
                     mimeData->setData(RingMimes::PHONENUMBER , ct->phoneNumbers()[0]->toHash().toUtf8());
                  }
                  mimeData->setData(RingMimes::CONTACT , ct->uid());
               }
               return mimeData;
               } break;
            case ContactTreeNode::NodeType::CONTACTMETHOD: {
               //Phone number
               const QString text = data(index, Qt::DisplayRole).toString();
               const ContactTreeNode* n = static_cast<ContactTreeNode*>(index.internalPointer());
               if (n->m_pContactMethod) {
                  mimeData->setData(RingMimes::PLAIN_TEXT , text.toUtf8());
                  mimeData->setData(RingMimes::PHONENUMBER, n->m_pContactMethod->toHash().toUtf8());
                  return mimeData;
               }

               } break;
            case ContactTreeNode::NodeType::CATEGORY:
               return nullptr;
         };
      }
   }
   return mimeData;
}

///Return valid payload types
int CategorizedContactModel::acceptedPayloadTypes()
{
   return CallModel::DropPayloadType::CALL;
}



/*****************************************************************************
 *                                                                           *
 *                                  Helpers                                  *
 *                                                                           *
 ****************************************************************************/


QString CategorizedContactModelPrivate::category(const Person* ct) const {
   if (!ct)
      return QString();

   QString cat = ct->roleData(m_Role).toString();

   if (cat.size() && m_SortAlphabetical)
      cat = cat[0].toUpper();
   else if (!cat.size())
      cat = m_DefaultCategory;

   return cat;
}

void CategorizedContactModel::setRole(int role)
{
   if (role != d_ptr->m_Role) {
      d_ptr->m_Role = role;
      d_ptr->reloadCategories();
   }
}

void CategorizedContactModel::setSortAlphabetical(bool alpha)
{
   d_ptr->m_SortAlphabetical = alpha;
}

bool CategorizedContactModel::isSortAlphabetical() const
{
   return d_ptr->m_SortAlphabetical;
}

void CategorizedContactModel::setDefaultCategory(const QString& cat)
{
   d_ptr->m_DefaultCategory = cat;
}

QString CategorizedContactModel::defaultCategory() const
{
   return d_ptr->m_DefaultCategory;
}

#include <categorizedcontactmodel.moc>
