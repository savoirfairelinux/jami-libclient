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

//Ring
#include "dbus/configurationmanager.h"

NetworkInterfaceModel* NetworkInterfaceModel::m_spInstance = nullptr;

class NetworkInterfaceModelPrivate : public QObject {
   Q_OBJECT
public:
   QStringList m_Interfaces;
};

NetworkInterfaceModel::NetworkInterfaceModel() : QAbstractListModel(QCoreApplication::instance()),
d_ptr(new NetworkInterfaceModelPrivate())
{
   //TODO get updates from the daemon
   d_ptr->m_Interfaces = DBus::ConfigurationManager::instance().getAllIpInterfaceByName();
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

NetworkInterfaceModel* NetworkInterfaceModel::instance()
{
   if (!m_spInstance)
      m_spInstance = new NetworkInterfaceModel();
   return m_spInstance;
}

///Translate enum type to QModelIndex
// QModelIndex NetworkInterfaceModel::toIndex(NetworkInterfaceModel::Type type)
// {
//    return index(static_cast<int>(type),0,QModelIndex());
// }


#include <networkinterfacemodel.moc>