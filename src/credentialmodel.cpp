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

class CredentialModelPrivate
{
public:
   ///@struct CredentialData store credential information
   struct CredentialData {
      QString name    ;
      QString password;
      QString realm   ;
   };

   //Attributes
   QList<CredentialData*>     m_lCredentials;
   Account*                   m_pAccount    ;
   CredentialModel::EditState m_EditState   ;
   CredentialModel*           q_ptr         ;
   static Matrix2D<CredentialModel::EditState, CredentialModel::EditAction,CredModelFct> m_mStateMachine;

   //Callbacks
   void clear  ();
   void save   ();
   void reload ();
   void nothing();
   void modify ();

   //Helper
   inline void performAction(const CredentialModel::EditAction action);
};

#define CMP &CredentialModelPrivate
Matrix2D<CredentialModel::EditState, CredentialModel::EditAction,CredModelFct> CredentialModelPrivate::m_mStateMachine ={{
   /*                    SAVE         MODIFY        RELOAD        CLEAR      */
   /* LOADING  */ {{ CMP::nothing, CMP::nothing, CMP::reload, CMP::nothing  }},
   /* READY    */ {{ CMP::nothing, CMP::modify , CMP::reload, CMP::clear    }},
   /* MODIFIED */ {{ CMP::save   , CMP::nothing, CMP::reload, CMP::clear    }},
   /* OUTDATED */ {{ CMP::save   , CMP::nothing, CMP::reload, CMP::clear    }},
}};
#undef CMP

///Constructor
CredentialModel::CredentialModel(Account* acc) : QAbstractListModel(acc),
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
   foreach (CredentialModelPrivate::CredentialData* data, d_ptr->m_lCredentials) {
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

///Model data
QVariant CredentialModel::data(const QModelIndex& idx, int role) const {
   if (!idx.isValid())
      return QVariant();

   if (idx.column() == 0) {
      switch (role) {
         case Qt::DisplayRole:
            return QVariant(d_ptr->m_lCredentials[idx.row()]->name);
         case CredentialModel::Role::NAME:
            return d_ptr->m_lCredentials[idx.row()]->name;
         case CredentialModel::Role::PASSWORD:
            return d_ptr->m_lCredentials[idx.row()]->password;
         case CredentialModel::Role::REALM:
            return d_ptr->m_lCredentials[idx.row()]->realm;
         default:
            break;
      }
   }
   return QVariant();
}

///Number of credentials
int CredentialModel::rowCount(const QModelIndex& par) const {
   Q_UNUSED(par)
   return d_ptr->m_lCredentials.size();
}

///Model flags
Qt::ItemFlags CredentialModel::flags(const QModelIndex& idx) const {
   if (idx.column() == 0)
      return QAbstractItemModel::flags(idx) /*| Qt::ItemIsUserCheckable*/ | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
   return QAbstractItemModel::flags(idx);
}

///Set credential data
bool CredentialModel::setData( const QModelIndex& idx, const QVariant &value, int role) {
   if (!idx.isValid() || idx.row() > d_ptr->m_lCredentials.size()-1)
      return false;
   if (idx.column() == 0 && role == CredentialModel::Role::NAME) {
      d_ptr->m_lCredentials[idx.row()]->name = value.toString();
      emit dataChanged(idx, idx);
      this << EditAction::MODIFY;
      return true;
   }
   else if (idx.column() == 0 && role == CredentialModel::Role::PASSWORD) {
      if (d_ptr->m_lCredentials[idx.row()]->password != value.toString()) {
         d_ptr->m_lCredentials[idx.row()]->password = value.toString();
         emit dataChanged(idx, idx);
         this << EditAction::MODIFY;
         return true;
      }
   }
   else if (idx.column() == 0 && role == CredentialModel::Role::REALM) {
      d_ptr->m_lCredentials[idx.row()]->realm = value.toString();
      emit dataChanged(idx, idx);
      this << EditAction::MODIFY;
      return true;
   }
   return false;
}

///Add a new credential
QModelIndex CredentialModel::addCredentials()
{
   beginInsertRows(QModelIndex(), d_ptr->m_lCredentials.size()-1, d_ptr->m_lCredentials.size()-1);
   d_ptr->m_lCredentials << new CredentialModelPrivate::CredentialData;
   endInsertRows();
   emit dataChanged(index(d_ptr->m_lCredentials.size()-1,0), index(d_ptr->m_lCredentials.size()-1,0));
   this << EditAction::MODIFY;
   return index(d_ptr->m_lCredentials.size()-1,0);
}

///Remove credential at 'idx'
void CredentialModel::removeCredentials(const QModelIndex& idx)
{
   if (idx.isValid()) {
      beginRemoveRows(QModelIndex(), idx.row(), idx.row());
      d_ptr->m_lCredentials.removeAt(idx.row());
      endRemoveRows();
      emit dataChanged(idx, index(d_ptr->m_lCredentials.size()-1,0));
      this << EditAction::MODIFY;
   }
   else {
      qDebug() << "Failed to remove an invalid credential";
   }
}

///Remove everything
void CredentialModelPrivate::clear()
{
   foreach(CredentialModelPrivate::CredentialData* data2, m_lCredentials) {
      delete data2;
   }
   m_lCredentials.clear();
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
         const QModelIndex& idx = q_ptr->addCredentials();
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
   (this->*(m_mStateMachine[m_EditState][action]))();//FIXME don't use integer cast
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
