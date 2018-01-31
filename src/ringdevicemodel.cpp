/****************************************************************************
 *   Copyright (C) 2016-2018 Savoir-faire Linux                               *
 *   Author : Alexandre Viau <alexandre.viau@savoirfairelinux.com>          *
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
#include "ringdevice.h"
#include "ringdevicemodel.h"
#include "private/ringdevicemodel_p.h"

#include "dbus/configurationmanager.h"

//Qt
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QDirIterator>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QStandardPaths>

//Ring daemon
#include <account_const.h>

//Ring
#include "account.h"

RingDeviceModelPrivate::RingDeviceModelPrivate(RingDeviceModel* q,Account* a) : q_ptr(q),m_pAccount(a)
{
}

void RingDeviceModelPrivate::clearLines()
{
    q_ptr->beginRemoveRows(QModelIndex(),0,m_lRingDevices.size());
    qDeleteAll(m_lRingDevices);
    m_lRingDevices.clear();
    q_ptr->endRemoveRows();
}

void RingDeviceModelPrivate::reload()
{
    const MapStringString accountDevices = ConfigurationManager::instance().getKnownRingDevices(this->m_pAccount->id());
    reload(accountDevices);
}

void RingDeviceModelPrivate::reload(MapStringString accountDevices)
{
    clearLines();

    QMapIterator<QString, QString> i(accountDevices);
    while (i.hasNext()) {
        i.next();

        RingDevice* device = new RingDevice(i.key(), i.value());

        q_ptr->beginInsertRows(QModelIndex(),m_lRingDevices.size(), m_lRingDevices.size());
        m_lRingDevices << device;
        q_ptr->endInsertRows();
    }
}

RingDeviceModel::RingDeviceModel(Account* a) : QAbstractTableModel(a), d_ptr(new RingDeviceModelPrivate(this,a))
{
    d_ptr->reload();
}

RingDeviceModel::~RingDeviceModel()
{
    d_ptr->clearLines();
    delete d_ptr;
}

QVariant RingDeviceModel::data( const QModelIndex& index, int role) const
{
    Q_UNUSED(role);

    if (role == Qt::DisplayRole && index.isValid()) {

        RingDevice* device = nullptr;
        if (index.row() < d_ptr->m_lRingDevices.size())
            device = d_ptr->m_lRingDevices[index.row()];

        if (!device)
            return QVariant();

        return device->columnData(index.column());
    } else
        return QVariant();
}

QVariant RingDeviceModel::headerData( int section, Qt::Orientation ori, int role) const
{
   if (role == Qt::DisplayRole) {
      if (ori == Qt::Vertical)
         return section;

      switch (section) {
         case static_cast<int>(RingDevice::Column::Id):
            return tr("Id");
         case static_cast<int>(RingDevice::Column::Name):
            return tr("Name");
        default:
            return QVariant();
      }
   }
   return QVariant();
}


int RingDeviceModel::rowCount( const QModelIndex& parent) const
{
   return parent.isValid()?0:d_ptr->m_lRingDevices.size();
}

int RingDeviceModel::columnCount( const QModelIndex& parent) const
{
   return parent.isValid()?0:2;
}

int RingDeviceModel::size() const
{
   return d_ptr->m_lRingDevices.size();
}

Qt::ItemFlags RingDeviceModel::flags(const QModelIndex &index) const
{
    return index.isValid() ? (Qt::ItemIsEnabled | Qt::ItemIsSelectable) : Qt::NoItemFlags;
}
