/****************************************************************************
 *   Copyright (C) 2016 by Savoir-faire Linux                               *
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
#include <account.h>
#include <private/account_p.h>

typedef void (RingDeviceModelPrivate::*RingDeviceModelPrivateFct)();

class RingDeviceModelPrivate
{
public:
   RingDeviceModelPrivate(RingDeviceModel* q,Account* a);

   //Attributes
   Account*                   m_pAccount     ;
   QVector<RingDevice*>       m_lRingDevices ;
   RingDeviceModel*           q_ptr          ;

   void reload();
   void reload(MapStringString accountDevices);
   void clearLines();

};

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

RingDeviceModel::RingDeviceModel(Account* a) : QAbstractListModel(a), d_ptr(new RingDeviceModelPrivate(this,a))
{
    d_ptr->reload();
}

RingDeviceModel::~RingDeviceModel()
{
   delete d_ptr;
}

void RingDeviceModel::reload( const MapStringString& accountDevices )
{
    d_ptr->reload(accountDevices);
}

QVariant RingDeviceModel::data( const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount())
       return QVariant();

    return d_ptr->m_lRingDevices[index.row()]->roleData(role);
}

int RingDeviceModel::rowCount( const QModelIndex& parent) const
{
   return parent.isValid()?0:d_ptr->m_lRingDevices.size();
}

int RingDeviceModel::size() const
{
   return d_ptr->m_lRingDevices.size();
}
