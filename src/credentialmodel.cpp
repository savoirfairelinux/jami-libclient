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

//Dring
#include "dbus/configurationmanager.h"
#include <account_const.h>

class CredentialModelPrivate
{
public:
   ///@struct CredentialData store credential information
   struct CredentialData2 {
      QString          name    ;
      QString          password;
      QString          realm   ;
   };

   //Attributes
   QList<CredentialData2*> m_lCredentials;
   Account*                m_pAccount;
};

///Constructor
CredentialModel::CredentialModel(Account* acc) : QAbstractListModel(acc),
d_ptr(new CredentialModelPrivate())
{
   Q_ASSERT(acc);
   d_ptr->m_pAccount = acc;
   QHash<int, QByteArray> roles = roleNames();
   reload();
}

CredentialModel::~CredentialModel()
{
   foreach (CredentialModelPrivate::CredentialData2* data, d_ptr->m_lCredentials) {
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
            break;
         case CredentialModel::Role::NAME:
            return d_ptr->m_lCredentials[idx.row()]->name;
            break;
         case CredentialModel::Role::PASSWORD:
            return d_ptr->m_lCredentials[idx.row()]->password;
            break;
         case CredentialModel::Role::REALM:
            return d_ptr->m_lCredentials[idx.row()]->realm;
            break;
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
      return true;
   }
   else if (idx.column() == 0 && role == CredentialModel::Role::PASSWORD) {
      d_ptr->m_lCredentials[idx.row()]->password = value.toString();
      emit dataChanged(idx, idx);
      return true;
   }
   else if (idx.column() == 0 && role == CredentialModel::Role::REALM) {
      d_ptr->m_lCredentials[idx.row()]->realm = value.toString();
      emit dataChanged(idx, idx);
      return true;
   }
   return false;
}

///Add a new credential
QModelIndex CredentialModel::addCredentials()
{
   beginInsertRows(QModelIndex(), d_ptr->m_lCredentials.size()-1, d_ptr->m_lCredentials.size()-1);
   d_ptr->m_lCredentials << new CredentialModelPrivate::CredentialData2;
   endInsertRows();
   emit dataChanged(index(d_ptr->m_lCredentials.size()-1,0), index(d_ptr->m_lCredentials.size()-1,0));
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
   }
   else {
      qDebug() << "Failed to remove an invalid credential";
   }
}

///Remove everything
void CredentialModel::clear()
{
   foreach(CredentialModelPrivate::CredentialData2* data2, d_ptr->m_lCredentials) {
      delete data2;
   }
   d_ptr->m_lCredentials.clear();
}

///Save all credentials
void CredentialModel::save()
{
   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
   VectorMapStringString toReturn;
   for (int i=0; i < rowCount();i++) {
      const QModelIndex& idx = index(i,0);
      MapStringString credentialData;
      QString user  = data(idx,CredentialModel::Role::NAME).toString();
      QString realm = data(idx,CredentialModel::Role::REALM).toString();
      if (user.isEmpty()) {
         user = d_ptr->m_pAccount->username();
         setData(idx,user,CredentialModel::Role::NAME);
      }
      if (realm.isEmpty()) {
         realm = '*';
         setData(idx,realm,CredentialModel::Role::REALM);
      }
      credentialData[ DRing::Account::ConfProperties::USERNAME ] = user;
      credentialData[ DRing::Account::ConfProperties::PASSWORD ] = data(idx,CredentialModel::Role::PASSWORD).toString();
      credentialData[ DRing::Account::ConfProperties::REALM    ] = realm;
      toReturn << credentialData;
   }
   configurationManager.setCredentials(d_ptr->m_pAccount->id(),toReturn);
}

///Reload credentials from DBUS
void CredentialModel::reload()
{
   if (!d_ptr->m_pAccount->isNew()) {
      clear();
      ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
      const VectorMapStringString credentials = configurationManager.getCredentials(d_ptr->m_pAccount->id());
      for (int i=0; i < credentials.size(); i++) {
         const QModelIndex& idx = addCredentials();
         setData(idx,credentials[i][ DRing::Account::ConfProperties::USERNAME ],CredentialModel::Role::NAME    );
         setData(idx,credentials[i][ DRing::Account::ConfProperties::PASSWORD ],CredentialModel::Role::PASSWORD);
         setData(idx,credentials[i][ DRing::Account::ConfProperties::REALM    ],CredentialModel::Role::REALM   );
      }
   }
}
