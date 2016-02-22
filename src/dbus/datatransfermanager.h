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
#pragma once

#ifdef ENABLE_LIBWRAP
#include "../qtwrapper/datatransfermanager_wrap.h"
#else
#include <datatransfer_interface.h> // TODO: forward declare structs instead?
#endif
#include <typedefs.h>

#ifndef ENABLE_LIBWRAP

///Wrap DataTransferManagerDBusInterface
class DataTransferManagerInterface: public QObject
{
Q_OBJECT
public:
    DataTransferManagerInterface();
    ~DataTransferManagerInterface() {}

public Q_SLOTS: // METHODS
    QString connectToPeer(const QString& accountId, const QString& peerUri);

    bool dataConnectionInfo(const QString& id, DRing::DataConnectionInfo& info);

    bool closeDataConnection(const QString& id);

    QString sendFile(const QString& accountId, const QString& peerUri, const QString& filename, const QString& displayname);

    bool dataTransferInfo(const QString& id, DRing::DataTransferInfo& info);

    std::streamsize dataTransferSentBytes(const QString& id);

    bool cancelDataTransfer(const QString& id);

    void acceptFileTransfer(const QString& id, const QString& pathname);

    // slots to forward signals from DBus interface
    void dataConnectionStatus(const QString& /*connectionID*/, int /*code*/);
    void dataTransferStatus(const QString& /*transferID*/, int /*code*/);

Q_SIGNALS: // SIGNALS
    void dataConnectionStatusChanged(const QString& /*connectionID*/, int /*code*/);
    void dataTransferStatusChanged(const QString& /*transferID*/, int /*code*/);
};

#endif

namespace DataTransferManager {

///Singleton to access the DataTransferManager interface
DataTransferManagerInterface& LIB_EXPORT instance();

}
