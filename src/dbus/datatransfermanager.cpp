/****************************************************************************
 *   Copyright (C) 2016 by Savoir-faire Linux                               *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com>      *
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
#include "datatransfermanager.h"

<<<<<<< HEAD
#include "../globalinstances.h"
#include "../interfaces/dbuserrorhandleri.h"

DataTransferManagerInterface& DataTransferManager::instance()
{
#ifdef ENABLE_LIBWRAP
    static auto interface = new DataTransferManagerInterface();
#else
    if (!dbus_metaTypeInit) registerCommTypes();
    static auto interface = new DataTransferManagerInterface("cx.ring.Ring",
                                                            "/cx/ring/Ring/DataTransferManager",
                                                            QDBusConnection::sessionBus());
    if(!interface->connection().isConnected()) {
        GlobalInstances::dBusErrorHandler().connectionError(
            "Error : dring not connected. Service " + interface->service() + " not connected. From configuration manager interface."
        );
    }
    if (!interface->isValid()) {
        GlobalInstances::dBusErrorHandler().invalidInterfaceError(
            "Error : dring is not available, make sure it is running"
        );
    }
#endif
    return *interface;
}
=======
#ifndef ENABLE_LIBWRAP

//Ring
#include "datatransferdbusmanager.h"
#include "datatransfer_dbus_interface.h"

//DRing
#include <datatransfer_interface.h>

#endif

DataTransferManagerInterface& DataTransferManager::instance()
{
    static auto interface = new DataTransferManagerInterface();
    return *interface;
}

#ifndef ENABLE_LIBWRAP

DataTransferManagerInterface::DataTransferManagerInterface()
{
    // forward the signals
    connect(&DataTransferDBusManager::instance(), &DataTransferManagerDBusInterface::dataConnectionStatus,
        this, &DataTransferManagerInterface::dataConnectionStatus);
    connect(&DataTransferDBusManager::instance(), &DataTransferManagerDBusInterface::dataTransferStatus,
        this, &DataTransferManagerInterface::dataTransferStatus);

}

QString
DataTransferManagerInterface::connectToPeer(const QString& accountId, const QString& peerUri)
{
    return DataTransferDBusManager::instance().connectToPeer(accountId, peerUri);
}

bool
DataTransferManagerInterface::dataConnectionInfo(const QString& id, DRing::DataConnectionInfo& info)
{
    MapStringString result = DataTransferDBusManager::instance().getDataConnectionInfo(id);
    if (result.isEmpty())
        return false;

    // TODO: check if the map actually contains the value
    info.account = result.value(DRing::DataTransfer::ACCOUNT).toStdString();
    info.peer    = result.value(DRing::DataTransfer::PEER).toStdString();
    info.code    = result.value(DRing::DataTransfer::CODE).toInt(); // TODO: check if converstion succeeds
    return true;
}

bool
DataTransferManagerInterface::closeDataConnection(const QString& id)
{
    return DataTransferDBusManager::instance().closeDataConnection(id);
}

QString
DataTransferManagerInterface::sendFile(const QString& accountId, const QString& peerUri, const QString& filename, const QString& displayname)
{
    return DataTransferDBusManager::instance().sendFile(accountId, peerUri, filename, displayname);
}

bool
DataTransferManagerInterface::dataTransferInfo(const QString& id, DRing::DataTransferInfo& info)
{
    MapStringString result = DataTransferDBusManager::instance().getDataTransferInfo(id);
    if (result.isEmpty())
        return false;

    // TODO: check if the map actually contains the value
    info.connectionId = result.value(DRing::DataTransfer::CONNECTION_ID).toStdString();
    info.name         = result.value(DRing::DataTransfer::NAME).toStdString();
    info.size         = result.value(DRing::DataTransfer::SIZE).toInt(); // TODO: check if converstion succeeds
    info.code         = result.value(DRing::DataTransfer::CODE).toInt(); // TODO: check if converstion succeeds
    return true;
}

std::streamsize
DataTransferManagerInterface::dataTransferSentBytes(const QString& id)
{
    return DataTransferDBusManager::instance().getDataTransferSentBytes(id);
}

bool
DataTransferManagerInterface::cancelDataTransfer(const QString& id)
{
    return DataTransferDBusManager::instance().cancelDataTransfer(id);
}

void
DataTransferManagerInterface::acceptFileTransfer(const QString& id, const QString& pathname)
{
    DataTransferDBusManager::instance().acceptFileTransfer(id, pathname);
}

void
DataTransferManagerInterface::dataConnectionStatus(const QString& connectionID, int code)
{
    emit dataConnectionStatusChanged(connectionID, code);
}

void
DataTransferManagerInterface::dataTransferStatus(const QString& transferID, int code)
{
    emit dataTransferStatusChanged(transferID, code);
}

#endif
>>>>>>> a6d9f31... datatransfer: add daemon API
