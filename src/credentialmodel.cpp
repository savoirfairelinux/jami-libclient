/****************************************************************************
 *   Copyright (C) 2012-2017 Savoir-faire Linux                          *
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
#include <QtCore/QItemSelectionModel>

//Ring
#include "account.h"
#include "private/matrixutils.h"

//Dring
#include "dbus/configurationmanager.h"
#include <account_const.h>

typedef void (CredentialModelPrivate::*CredModelFct)();

struct CredentialNode final
{
   Credential*               m_pCredential  {nullptr};
   CredentialNode*           m_pParent      {nullptr};
   QString*                  m_CategoryName {nullptr};
   QVector<CredentialNode*>  m_lChildren             ;

   enum class Level {
      CATEGORY,
      CREDENTIAL
   };

   short int m_Index = {-1};
   Level m_Level;
};

/**
 * Display the types of credentials that can be added
 * This depend on the account type and settings
 */
class NewCredentialTypeModel final : public QAbstractListModel
{
   Q_OBJECT
   friend class CredentialModel;
public:
   explicit NewCredentialTypeModel(Account* a);
   virtual ~NewCredentialTypeModel();

   virtual QVariant      data    ( const QModelIndex& index, int role = Qt::DisplayRole        ) const override;
   virtual int           rowCount( const QModelIndex& parent = QModelIndex()                   ) const override;
   virtual Qt::ItemFlags flags   ( const QModelIndex& index                                    ) const override;
   virtual bool          setData ( const QModelIndex& index, const QVariant &value, int role   )       override;

private:
   Account* m_pAccount;
   QItemSelectionModel* m_pSelectionModel {nullptr};
   QStringList m_lValues { QStringLiteral("SIP"),QStringLiteral("STUN"),QStringLiteral("TURN")};
   static const Matrix1D<Credential::Type, int> m_smMaximumCount;
   static const Matrix2D<Credential::Type, Account::Protocol, bool> m_smAvailableInProtocol;
};

class CredentialModelPrivate final
{
public:

   //Attributes
   QList<CredentialNode*>     m_lCategories  ;
   Account*                   m_pAccount     ;
   CredentialModel::EditState m_EditState    ;
   CredentialModel*           q_ptr          ;
   uint                       m_TopLevelCount = {3};
   static Matrix2D<CredentialModel::EditState, CredentialModel::EditAction,CredModelFct> m_mStateMachine;
   CredentialNode* m_pSipCat   {nullptr};
   CredentialNode* m_pTurnCat  {nullptr};
   CredentialNode* m_pStunCat  {nullptr};
   NewCredentialTypeModel* m_pTypeModel {nullptr};

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
      delete data;
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

   return createIndex(node->m_pParent->m_Index,0,node->m_pParent);
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
       || row >= node->m_lChildren.size())
         return QModelIndex();

      return createIndex(row, column, node->m_lChildren.at(row));
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
               return *node->m_CategoryName;
         }
         break;
      case CredentialNode::Level::CREDENTIAL:
         switch (role) {
            case Qt::DisplayRole:
            case CredentialModel::Role::NAME:
               return node->m_pCredential->username();
            case CredentialModel::Role::PASSWORD:
               return node->m_pCredential->password();
            case CredentialModel::Role::REALM:
               return node->m_pCredential->realm();
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

   return d_ptr->m_lCategories[par.row()]->m_lChildren.size();
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
      node->m_pCredential->setUsername(value.toString());
      emit dataChanged(idx, idx);
      this << EditAction::MODIFY;
      return true;
   }
   else if (idx.column() == 0 && role == CredentialModel::Role::PASSWORD) {
      if (node->m_pCredential->password() != value.toString()) {
         node->m_pCredential->setPassword(value.toString());
         emit dataChanged(idx, idx);
         this << EditAction::MODIFY;
         return true;
      }
   }
   else if (idx.column() == 0 && role == CredentialModel::Role::REALM) {
      node->m_pCredential->setRealm(value.toString());
      emit dataChanged(idx, idx);
      this << EditAction::MODIFY;
      return true;
   }
   return false;
}

CredentialNode* CredentialModelPrivate::createCat(const QString& name)
{
   CredentialNode* n = new CredentialNode();
   n->m_Level = CredentialNode::Level::CATEGORY;
   n->m_Index = q_ptr->rowCount();
   n->m_CategoryName = new QString(name);

   q_ptr->beginInsertRows(QModelIndex(),m_lCategories.size(),0);
   m_lCategories << n;
   q_ptr->endInsertRows();

   return n;
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
         if (!m_pStunCat)
            m_pStunCat = createCat(QStringLiteral("STUN"));
         return m_pStunCat;
      case Credential::Type::COUNT__:
         break;
   }

   return nullptr;
}

///Use the selectionmodel to get the type
QModelIndex CredentialModel::addCredentials()
{
   Credential::Type type = Credential::Type::SIP;

   if (d_ptr->m_pTypeModel && d_ptr->m_pTypeModel->m_pSelectionModel
     && d_ptr->m_pTypeModel->m_pSelectionModel->currentIndex().isValid())
      type = static_cast<Credential::Type>(
         d_ptr->m_pTypeModel->m_pSelectionModel->currentIndex().row()
      );

   return addCredentials(type);
}

///Add a new credential
QModelIndex CredentialModel::addCredentials(Credential::Type type)
{
   CredentialNode* par = d_ptr->getCategory(type);
   const int count = par->m_lChildren.size();
   const QModelIndex parIdx = index(par->m_Index,0);
   beginInsertRows(parIdx, count, count);

   CredentialNode* node = new CredentialNode();
   node->m_Level = CredentialNode::Level::CREDENTIAL;
   node->m_pCredential = new Credential(type);
   node->m_pParent = par;
   node->m_Index = par->m_lChildren.size();
   par->m_lChildren.append(node);

   connect(node->m_pCredential, &Credential::changed,[this, node, par, type]() {
      const QModelIndex parIdx = index(par->m_Index,0);
      const QModelIndex idx    = index(node->m_Index,0,parIdx);
      emit dataChanged(idx, idx);

      if (!node->m_Index) {
         Credential* c = node->m_pCredential;
         emit primaryCredentialChanged(type, c);
      }
   });

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

      for (int i = node->m_Index+1; i < node->m_pParent->m_lChildren.size();i++) {
         node->m_pParent->m_lChildren.at(i)->m_Index--;
      }
      node->m_pParent->m_lChildren.removeAt(node->m_Index);
      delete node;
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
   m_pSipCat  = nullptr;
   m_pTurnCat = nullptr;
   m_pSipCat  = nullptr;
   foreach(CredentialNode* data2, m_lCategories) {
      delete data2;
   }
   m_lCategories.clear();
   q_ptr->endRemoveRows();
   m_EditState = CredentialModel::EditState::READY;
}

///Save all credentials
void CredentialModelPrivate::save()
{
   ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();

   //SIP creds
   if (m_pSipCat) {
      VectorMapStringString toReturn;

      foreach (CredentialNode* n, m_pSipCat->m_lChildren) {
         Credential* cred = n->m_pCredential;

         if (cred->username().isEmpty())
            cred->setUsername(m_pAccount->username());

         if (cred->realm().isEmpty())
            cred->setRealm(QStringLiteral("*"));

         toReturn << MapStringString {
            { DRing::Account::ConfProperties::USERNAME, cred->username() },
            { DRing::Account::ConfProperties::PASSWORD, cred->password() },
            { DRing::Account::ConfProperties::REALM   , cred->realm   () },
         };
      }
      configurationManager.setCredentials(m_pAccount->id(),toReturn);
   }

   //TURN creds
   if (m_pTurnCat) {
      foreach (CredentialNode* n, m_pTurnCat->m_lChildren) {
         Credential* cred = n->m_pCredential;

         m_pAccount->setAccountProperty(DRing::Account::ConfProperties::TURN::SERVER_UNAME , cred->username());
         m_pAccount->setAccountProperty(DRing::Account::ConfProperties::TURN::SERVER_PWD   , cred->password());
         m_pAccount->setAccountProperty(DRing::Account::ConfProperties::TURN::SERVER_REALM , cred->realm   ());
      }
   }

   m_EditState = CredentialModel::EditState::READY;
}

///Reload credentials from DBUS
void CredentialModelPrivate::reload()
{
   if (!m_pAccount->isNew()) {
      clear();
      m_EditState = CredentialModel::EditState::LOADING;

      ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();

      //SIP
      const VectorMapStringString credentials = configurationManager.getCredentials(m_pAccount->id());
      for (int i=0; i < credentials.size(); i++) {
         const QModelIndex& idx = q_ptr->addCredentials(Credential::Type::SIP);
         q_ptr->setData(idx,credentials[i][ DRing::Account::ConfProperties::USERNAME ],CredentialModel::Role::NAME    );
         q_ptr->setData(idx,credentials[i][ DRing::Account::ConfProperties::PASSWORD ],CredentialModel::Role::PASSWORD);
         q_ptr->setData(idx,credentials[i][ DRing::Account::ConfProperties::REALM    ],CredentialModel::Role::REALM   );
      }

      //TURN
      const QModelIndex& idx = q_ptr->addCredentials(Credential::Type::TURN);
      const QString usern = m_pAccount->accountDetail(DRing::Account::ConfProperties::TURN::SERVER_UNAME);
      const QString passw = m_pAccount->accountDetail(DRing::Account::ConfProperties::TURN::SERVER_PWD  );
      const QString realm = m_pAccount->accountDetail(DRing::Account::ConfProperties::TURN::SERVER_REALM);

      if (!(usern.isEmpty() && passw.isEmpty() && realm.isEmpty())) {
         q_ptr->setData(idx, usern, CredentialModel::Role::NAME    );
         q_ptr->setData(idx, passw, CredentialModel::Role::PASSWORD);
         q_ptr->setData(idx, realm, CredentialModel::Role::REALM   );
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

/*
 * Return the primary credential's set of specified type
 * @return credential object, new empty one if none existed.
 */
Credential* CredentialModel::primaryCredential(Credential::Type type)
{
   switch(type) {
      case Credential::Type::STUN:
         if (!d_ptr->m_pStunCat || !d_ptr->m_pStunCat->m_lChildren.size())
            addCredentials(Credential::Type::STUN);
         return d_ptr->m_pStunCat->m_lChildren.first()->m_pCredential;
         break;
      case Credential::Type::TURN:
         if (!d_ptr->m_pTurnCat || !d_ptr->m_pTurnCat->m_lChildren.size())
            addCredentials(Credential::Type::TURN);
         return d_ptr->m_pTurnCat->m_lChildren.first()->m_pCredential;
         break;
      case Credential::Type::SIP:
         if (!d_ptr->m_pSipCat || !d_ptr->m_pSipCat->m_lChildren.size())
            addCredentials(Credential::Type::SIP);
         return d_ptr->m_pSipCat->m_lChildren.first()->m_pCredential;
         break;
      case Credential::Type::COUNT__:
         break;
   }

   return nullptr;
}

/**
 * NewCredentialTypeModel
 */

///Is a credential type available for an account protocol
const Matrix2D<Credential::Type, Account::Protocol, bool> NewCredentialTypeModel::m_smAvailableInProtocol = {
   /*                          SIP   RING */
   { Credential::Type::SIP , {{true, false}}},
   { Credential::Type::STUN, {{true, true }}},
   { Credential::Type::TURN, {{true, true }}},
};

///The maximum number of credentials (-1 = inf), this could be protocol dependent
const Matrix1D<Credential::Type, int> NewCredentialTypeModel::m_smMaximumCount = {
   { Credential::Type::SIP , -1 },
   { Credential::Type::STUN,  1 },
   { Credential::Type::TURN,  1 },
};

NewCredentialTypeModel::NewCredentialTypeModel(Account* a) : m_pAccount(a)
{

}

NewCredentialTypeModel::~NewCredentialTypeModel()
{}

QVariant NewCredentialTypeModel::data( const QModelIndex& index, int role ) const
{
   if (!index.isValid())
      return QVariant();

   switch (role) {
      case Qt::DisplayRole:
         return m_lValues[index.row()];
   }

   return QVariant();
}

int NewCredentialTypeModel::rowCount( const QModelIndex& parent ) const
{
   return parent.isValid() ? 0 : m_lValues.size();
}

///Only enabled the available types
Qt::ItemFlags NewCredentialTypeModel::flags( const QModelIndex& index ) const
{
   if (!index.isValid())
      return Qt::NoItemFlags;

   const Credential::Type t = static_cast<Credential::Type>(index.row());

   bool avail = m_smAvailableInProtocol[t][m_pAccount->protocol()];

#if 0 //TODO it mostly work, but make developement/testing harder, to enable in the last patch
   switch(t) {
      case Credential::Type::STUN:
         avail &= m_pAccount->isSipStunEnabled();
         break;
      case Credential::Type::TURN:
         avail &= m_pAccount->isTurnEnabled();
         break;
      case Credential::Type::SIP:
      case Credential::Type::COUNT__:
         break;
   }
#endif

   return avail ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::NoItemFlags;
}

bool NewCredentialTypeModel::setData( const QModelIndex& index, const QVariant &value, int role   )
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}

QAbstractItemModel* CredentialModel::availableTypeModel() const
{
   if (!d_ptr->m_pTypeModel)
      d_ptr->m_pTypeModel = new NewCredentialTypeModel(d_ptr->m_pAccount);

   return d_ptr->m_pTypeModel;
}

QItemSelectionModel* CredentialModel::availableTypeSelectionModel() const
{
   if (!static_cast<NewCredentialTypeModel*>(availableTypeModel())->m_pSelectionModel) {
      d_ptr->m_pTypeModel->m_pSelectionModel = new QItemSelectionModel(d_ptr->m_pTypeModel);
      for (int i=0; i < d_ptr->m_pTypeModel->rowCount(); i++) {
         const QModelIndex idx = d_ptr->m_pTypeModel->index(i,0);
         if (idx.flags() & Qt::ItemIsSelectable) {
            d_ptr->m_pTypeModel->m_pSelectionModel->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
            break;
         }
      }
   }

   return d_ptr->m_pTypeModel->m_pSelectionModel;
}

#include <credentialmodel.moc>
