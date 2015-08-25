/****************************************************************************
 *   Copyright (C) 2012-2015 by Savoir-Faire Linux                          *
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
#include "credentialmodel.h"

//Qt
#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>

//Ring
#include <account.h>
#include <private/matrixutils.h>

//Dring
#include "dbus/configurationmanager.h"
#include <account_const.h>

typedef void (CredentialModelPrivate::*CredModelFct)();

struct CredentialNode final
{
   union {
      struct {
         Credential* m_pCredential = {nullptr};
         CredentialNode* m_pParent = {nullptr};
      } m_Cred;
      struct {
         QString* m_CategoryName = {nullptr};
         QVector<CredentialNode*>* m_lChildren;
      } m_Category;
   } m_uContent;

   enum class Level {
      CATEGORY,
      CREDENTIAL
   };

   short int m_Index = {-1};
   Level m_Level;
};
void free_node(CredentialNode* n);

class CredentialModelPrivate
{
public:

   //Attributes
   QList<CredentialNode*>     m_lCategories  ;
   Account*                   m_pAccount     ;
   CredentialModel::EditState m_EditState    ;
   CredentialModel*           q_ptr          ;
   uint                       m_TopLevelCount = {3};
   static Matrix2D<CredentialModel::EditState, CredentialModel::EditAction,CredModelFct> m_mStateMachine;
   CredentialNode* m_pSipCat  = {nullptr};
   CredentialNode* m_pTurnCat = {nullptr};
   CredentialNode* m_pStunCat = {nullptr};

   //Callbacks
   void clear  ();
   void save   ();
   void reload ();
   void nothing();
   void modify ();

   //Helper
   inline void performAction(const CredentialModel::EditAction action);
   CredentialNode* getCategory(Credential::Type type);
   CredentialNode* createCat(const QString& name);
};

#define CMP &CredentialModelPrivate
Matrix2D<CredentialModel::EditState, CredentialModel::EditAction,CredModelFct> CredentialModelPrivate::m_mStateMachine ={{
   /*                                              SAVE         MODIFY        RELOAD        CLEAR       */
   { CredentialModel::EditState::LOADING  , {{ CMP::nothing, CMP::nothing, CMP::reload, CMP::nothing  }}},
   { CredentialModel::EditState::READY    , {{ CMP::nothing, CMP::modify , CMP::reload, CMP::clear    }}},
   { CredentialModel::EditState::MODIFIED , {{ CMP::save   , CMP::nothing, CMP::reload, CMP::clear    }}},
   { CredentialModel::EditState::OUTDATED , {{ CMP::save   , CMP::nothing, CMP::reload, CMP::clear    }}},
}};
#undef CMP

///Constructor
CredentialModel::CredentialModel(Account* acc) : QAbstractItemModel(acc),
d_ptr(new CredentialModelPrivate())
{
   Q_ASSERT(acc);
   d_ptr->m_EditState = CredentialModel::EditState::LOADING;
   d_ptr->m_pAccount  = acc;
   d_ptr->q_ptr       = this;
   QHash<int, QByteArray> roles = roleNames();
   this << EditAction::RELOAD;
   d_ptr->m_EditState = CredentialModel::EditState::READY;
}

CredentialModel::~CredentialModel()
{
   foreach (CredentialNode* data, d_ptr->m_lCategories) {
      free(data); //delete doesn't support non-trivial structures
   }
}

QHash<int,QByteArray> CredentialModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
      roles.insert(CredentialModel::Role::NAME    ,QByteArray("name"));
      roles.insert(CredentialModel::Role::PASSWORD,QByteArray("password"));
      roles.insert(CredentialModel::Role::REALM   ,QByteArray("realm"));
   }
   return roles;
}

///Get the parent index
QModelIndex CredentialModel::parent( const QModelIndex& index) const
{
   if (!index.isValid())
      return QModelIndex();

   const CredentialNode* node = static_cast<CredentialNode*>(index.internalPointer());

   if ((!node) || node->m_Level == CredentialNode::Level::CATEGORY)
      return QModelIndex();

   return createIndex(node->m_uContent.m_Cred.m_pParent->m_Index,0,node->m_uContent.m_Cred.m_pParent);
}

///Get the column count
int CredentialModel::columnCount( const QModelIndex& parent) const
{
   Q_UNUSED(parent)
   return 1;
}

///Create the indexes
QModelIndex CredentialModel::index( int row, int column, const QModelIndex& parent) const
{
   if (column)
      return QModelIndex();

   if (parent.isValid()) {
      const CredentialNode* node = static_cast<CredentialNode*>(parent.internalPointer());

      if (node->m_Level != CredentialNode::Level::CATEGORY
       || row >= node->m_uContent.m_Category.m_lChildren->size())
         return QModelIndex();

      return createIndex(row, column, node->m_uContent.m_Category.m_lChildren->at(row));
   }

   if (row >= d_ptr->m_lCategories.size())
      return QModelIndex();

   return createIndex(row, column, d_ptr->m_lCategories[row]);
}

///Model data
QVariant CredentialModel::data(const QModelIndex& idx, int role) const {
   if (!idx.isValid())
      return QVariant();

   const CredentialNode* node = static_cast<CredentialNode*>(idx.internalPointer());

   switch (node->m_Level) {
      case CredentialNode::Level::CATEGORY:
         switch(role) {
            case Qt::DisplayRole:
               return *node->m_uContent.m_Category.m_CategoryName;
         }
         break;
      case CredentialNode::Level::CREDENTIAL:
         switch (role) {
            case Qt::DisplayRole:
            case CredentialModel::Role::NAME:
               return node->m_uContent.m_Cred.m_pCredential->username();
            case CredentialModel::Role::PASSWORD:
               return node->m_uContent.m_Cred.m_pCredential->password();
            case CredentialModel::Role::REALM:
               return node->m_uContent.m_Cred.m_pCredential->realm();
            default:
               break;
         }
         break;
   }
   return QVariant();
}

///Number of credentials
int CredentialModel::rowCount(const QModelIndex& par) const {
   if (!par.isValid())
      return d_ptr->m_lCategories.size();

   const CredentialNode* node = static_cast<CredentialNode*>(par.internalPointer());

   if (node->m_Level == CredentialNode::Level::CREDENTIAL)
      return 0;

   return d_ptr->m_lCategories[par.row()]->m_uContent.m_Category.m_lChildren->size();
}

///Model flags
Qt::ItemFlags CredentialModel::flags(const QModelIndex& idx) const {
   if (!idx.isValid())
      return Qt::NoItemFlags;

   const CredentialNode* node = static_cast<CredentialNode*>(idx.internalPointer());

   //Categories cannot be selected
   switch (node->m_Level) {
      case CredentialNode::Level::CATEGORY:
         return Qt::ItemIsEnabled;
      case CredentialNode::Level::CREDENTIAL:
         return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
         //TODO make turn/stun disabled is account doesn't support/disable it
   }

   return Qt::NoItemFlags;
}

///Set credential data
bool CredentialModel::setData( const QModelIndex& idx, const QVariant &value, int role) {
   if (!idx.isValid())
      return false;

   const CredentialNode* node = static_cast<CredentialNode*>(idx.internalPointer());

   if (node->m_Level == CredentialNode::Level::CATEGORY)
      return false;

   if (idx.column() == 0 && role == CredentialModel::Role::NAME) {
      node->m_uContent.m_Cred.m_pCredential->setUsername(value.toString());
      emit dataChanged(idx, idx);
      this << EditAction::MODIFY;
      return true;
   }
   else if (idx.column() == 0 && role == CredentialModel::Role::PASSWORD) {
      if (node->m_uContent.m_Cred.m_pCredential->password() != value.toString()) {
         node->m_uContent.m_Cred.m_pCredential->setPassword(value.toString());
         emit dataChanged(idx, idx);
         this << EditAction::MODIFY;
         return true;
      }
   }
   else if (idx.column() == 0 && role == CredentialModel::Role::REALM) {
      node->m_uContent.m_Cred.m_pCredential->setRealm(value.toString());
      emit dataChanged(idx, idx);
      this << EditAction::MODIFY;
      return true;
   }
   return false;
}

CredentialNode* CredentialModelPrivate::createCat(const QString& name)
{
   CredentialNode* n = (CredentialNode*) malloc(sizeof(CredentialNode));
   n->m_Level = CredentialNode::Level::CATEGORY;
   n->m_Index = q_ptr->rowCount();
   n->m_uContent.m_Category.m_CategoryName = new QString(name);
   n->m_uContent.m_Category.m_lChildren = new QVector<CredentialNode*>();

   q_ptr->beginInsertRows(QModelIndex(),m_lCategories.size(),0);
   m_lCategories << n;
   q_ptr->endInsertRows();

   return n;
}

void free_node(CredentialNode* n)
{
   switch (n->m_Level) {
      case CredentialNode::Level::CATEGORY:
         delete n->m_uContent.m_Category.m_CategoryName;
         delete n->m_uContent.m_Category.m_lChildren;
         break;
      case CredentialNode::Level::CREDENTIAL:
         delete n->m_uContent.m_Cred.m_pCredential;
         break;
   }
   free(n);
}

CredentialNode* CredentialModelPrivate::getCategory(Credential::Type type)
{
   switch (type) {
      case Credential::Type::SIP:
         if (!m_pSipCat)
            m_pSipCat = createCat(QStringLiteral("SIP"));
         return m_pSipCat;
      case Credential::Type::TURN:
         if (!m_pTurnCat)
            m_pTurnCat = createCat(QStringLiteral("TURN"));
         return m_pTurnCat;
      case Credential::Type::STUN:
         if (m_pStunCat)
            m_pStunCat = createCat(QStringLiteral("STUN"));
         return m_pStunCat;
   }

   return nullptr;
}

///Add a new credential
QModelIndex CredentialModel::addCredentials(Credential::Type type)
{
   CredentialNode* par = d_ptr->getCategory(type);
   const int count = par->m_uContent.m_Category.m_lChildren->size();
   const QModelIndex parIdx = index(par->m_Index,0);
   beginInsertRows(parIdx, count, count);

   CredentialNode* node = (CredentialNode*) malloc(sizeof(CredentialNode));
   node->m_Level = CredentialNode::Level::CREDENTIAL;
   node->m_uContent.m_Cred.m_pCredential = new Credential(type);
   node->m_uContent.m_Cred.m_pParent = par;
   node->m_Index = par->m_uContent.m_Category.m_lChildren->size();
   par->m_uContent.m_Category.m_lChildren->append(node);

   endInsertRows();

   this << EditAction::MODIFY;

   return index(count, 0, parIdx);
}

///Remove credential at 'idx'
void CredentialModel::removeCredentials(const QModelIndex& idx)
{
   if (idx.isValid() && idx.parent().isValid()) {
      beginRemoveRows(idx.parent(), idx.row(), idx.row());

      CredentialNode* node = static_cast<CredentialNode*>(idx.internalPointer());

      for (int i = node->m_Index+1; i < node->m_uContent.m_Cred.m_pParent->m_uContent.m_Category.m_lChildren->size();i++) {
         node->m_uContent.m_Cred.m_pParent->m_uContent.m_Category.m_lChildren->at(i)->m_Index--;
      }
      node->m_uContent.m_Cred.m_pParent->m_uContent.m_Category.m_lChildren->removeAt(node->m_Index);
      free_node(node);
      endRemoveRows();

      this << EditAction::MODIFY;
   }
   else {
      qDebug() << "Failed to remove an invalid credential";
   }
}

///Remove everything
void CredentialModelPrivate::clear()
{
   q_ptr->beginRemoveRows(QModelIndex(),0,q_ptr->rowCount());
   foreach(CredentialNode* data2, m_lCategories) {
      free_node(data2);
   }
   m_lCategories.clear();
   q_ptr->endRemoveRows();
   m_EditState = CredentialModel::EditState::READY;
}

///Save all credentials
void CredentialModelPrivate::save()
{
   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
   VectorMapStringString toReturn;
   for (int i=0; i < q_ptr->rowCount();i++) {
      const QModelIndex& idx = q_ptr->index(i,0);
      MapStringString credentialData;
      QString user  = q_ptr->data(idx,CredentialModel::Role::NAME).toString();
      QString realm = q_ptr->data(idx,CredentialModel::Role::REALM).toString();
      if (user.isEmpty()) {
         user = m_pAccount->username();
         q_ptr->setData(idx,user,CredentialModel::Role::NAME);
      }
      if (realm.isEmpty()) {
         realm = '*';
         q_ptr->setData(idx,realm,CredentialModel::Role::REALM);
      }
      credentialData[ DRing::Account::ConfProperties::USERNAME ] = user;
      credentialData[ DRing::Account::ConfProperties::PASSWORD ] = q_ptr->data(idx,CredentialModel::Role::PASSWORD).toString();
      credentialData[ DRing::Account::ConfProperties::REALM    ] = realm;
      toReturn << credentialData;
   }
   configurationManager.setCredentials(m_pAccount->id(),toReturn);
   m_EditState = CredentialModel::EditState::READY;
}

///Reload credentials from DBUS
void CredentialModelPrivate::reload()
{
   if (!m_pAccount->isNew()) {
      clear();
      m_EditState = CredentialModel::EditState::LOADING;
      ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
      const VectorMapStringString credentials = configurationManager.getCredentials(m_pAccount->id());
      for (int i=0; i < credentials.size(); i++) {
         const QModelIndex& idx = q_ptr->addCredentials(Credential::Type::SIP);
         q_ptr->setData(idx,credentials[i][ DRing::Account::ConfProperties::USERNAME ],CredentialModel::Role::NAME    );
         q_ptr->setData(idx,credentials[i][ DRing::Account::ConfProperties::PASSWORD ],CredentialModel::Role::PASSWORD);
         q_ptr->setData(idx,credentials[i][ DRing::Account::ConfProperties::REALM    ],CredentialModel::Role::REALM   );
      }
   }
   m_EditState = CredentialModel::EditState::READY;
}

void CredentialModelPrivate::nothing()
{
   //nothing
}

void CredentialModelPrivate::modify()
{
   m_EditState = CredentialModel::EditState::MODIFIED;
   m_pAccount << Account::EditAction::MODIFY;
}

void CredentialModelPrivate::performAction(const CredentialModel::EditAction action)
{
   (this->*(m_mStateMachine[m_EditState][action]))();
}

/// anAccount << Call::EditAction::SAVE
CredentialModel* CredentialModel::operator<<(CredentialModel::EditAction& action)
{
   performAction(action);
   return this;
}

CredentialModel* operator<<(CredentialModel* a, CredentialModel::EditAction action)
{
   return (!a)?nullptr : (*a) << action;
}

///Change the current edition state
bool CredentialModel::performAction(const CredentialModel::EditAction action)
{
   CredentialModel::EditState curState = d_ptr->m_EditState;
   d_ptr->performAction(action);
   return curState != d_ptr->m_EditState;
}
