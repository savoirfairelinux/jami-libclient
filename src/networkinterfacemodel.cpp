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
#include "networkinterfacemodel.h"

//Qt
#include <QtCore/QCoreApplication>
#include <QtCore/QItemSelectionModel>

//Ring daemon
#include <account_const.h>

//Ring
#include "dbus/configurationmanager.h"
#include <account.h>
#include <private/account_p.h>

class NetworkInterfaceModelPrivate : public QObject {
   Q_OBJECT
public:
   NetworkInterfaceModelPrivate(NetworkInterfaceModel*);
   QStringList m_Interfaces;
   QItemSelectionModel* m_pSelectionModel;
   Account* m_pAccount;

   //Helper
   QString localInterface() const;
   void setLocalInterface(const QString& detail);

public Q_SLOTS:
   void slotSelectionChanged(const QModelIndex& idx, const QModelIndex& prev);

private:
   NetworkInterfaceModel* q_ptr;
};

NetworkInterfaceModelPrivate::NetworkInterfaceModelPrivate(NetworkInterfaceModel* parent): QObject(parent),q_ptr(parent),m_pSelectionModel(nullptr),m_pAccount(nullptr)
{
}

NetworkInterfaceModel::NetworkInterfaceModel(Account* a) : QAbstractListModel(QCoreApplication::instance()),
d_ptr(new NetworkInterfaceModelPrivate(this))
{
   //TODO get updates from the daemon
   d_ptr->m_pAccount = a;
   d_ptr->m_Interfaces = DBus::ConfigurationManager::instance().getAllIpInterfaceByName();
}

QHash<int,QByteArray> NetworkInterfaceModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;

   }*/
   return roles;
}

///Return the account local interface
QString NetworkInterfaceModelPrivate::localInterface() const
{
   return m_pAccount->d_ptr->accountDetail(DRing::Account::ConfProperties::LOCAL_INTERFACE);
}

///Set the local interface
void NetworkInterfaceModelPrivate::setLocalInterface(const QString& detail)
{
   m_pAccount->d_ptr->setAccountProperty(DRing::Account::ConfProperties::LOCAL_INTERFACE, detail);
}

//Model functions
QVariant NetworkInterfaceModel::data( const QModelIndex& index, int role) const
{
   if (role == Qt::DisplayRole)
      return d_ptr->m_Interfaces[index.row()];
   return QVariant();
}

int NetworkInterfaceModel::rowCount( const QModelIndex& parent ) const
{
   return parent.isValid()?0:d_ptr->m_Interfaces.size();
}

Qt::ItemFlags NetworkInterfaceModel::flags( const QModelIndex& index ) const
{
   if (!index.isValid()) return Qt::NoItemFlags;
   return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
}

bool NetworkInterfaceModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role )
   return false;
}

void NetworkInterfaceModelPrivate::slotSelectionChanged(const QModelIndex& idx, const QModelIndex& prev)
{
   Q_UNUSED(prev)
   setLocalInterface(idx.data(Qt::DisplayRole).toString());
}

QItemSelectionModel* NetworkInterfaceModel::selectionModel() const
{
   if (!d_ptr->m_pSelectionModel) {
      d_ptr->m_pSelectionModel = new QItemSelectionModel(const_cast<NetworkInterfaceModel*>(this));

      const QString currentInterface = d_ptr->localInterface();
      int idx = d_ptr->m_Interfaces.indexOf(currentInterface);

      //Some interfaces could be currently disconnected
      if (idx == -1) {
         idx = d_ptr->m_Interfaces.size();

         const_cast<NetworkInterfaceModel*>(this)->beginInsertRows(QModelIndex(), idx, idx);
         d_ptr->m_Interfaces << currentInterface;
         const_cast<NetworkInterfaceModel*>(this)->endInsertRows();
      }
      d_ptr->m_pSelectionModel->setCurrentIndex(index(idx,0), QItemSelectionModel::ClearAndSelect);

      connect(d_ptr->m_pSelectionModel, &QItemSelectionModel::currentChanged, d_ptr, &NetworkInterfaceModelPrivate::slotSelectionChanged);
   }

   return d_ptr->m_pSelectionModel;
}

///Translate enum type to QModelIndex
// QModelIndex NetworkInterfaceModel::toIndex(NetworkInterfaceModel::Type type)
// {
//    return index(static_cast<int>(type),0,QModelIndex());
// }


#include <networkinterfacemodel.moc>
