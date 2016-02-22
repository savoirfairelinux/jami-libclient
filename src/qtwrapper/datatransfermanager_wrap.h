/******************************************************************************
 *   Copyright (C) 2016 by Savoir-faire Linux                                 *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com>        *
 *                                                                            *
 *   This library is free software; you can redistribute it and/or            *
 *   modify it under the terms of the GNU Lesser General Public               *
 *   License as published by the Free Software Foundation; either             *
 *   version 2.1 of the License, or (at your option) any later version.       *
 *                                                                            *
 *   This library is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *   Lesser General Public License for more details.                          *
 *                                                                            *
 *   You should have received a copy of the Lesser GNU General Public License *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *****************************************************************************/
#pragma once

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtCore/QTimer>

#include <datatransfer_interface.h>

#include "typedefs.h"
#include "conversions_wrap.hpp"

/*
 * Proxy class for interface org.ring.Ring.DataTransferInterface
 */
class DataTransferManagerInterface: public QObject
{
    Q_OBJECT
public:

    std::map<std::string, std::shared_ptr<DRing::CallbackWrapperBase>> dataHandlers;

    DataTransferManagerInterface()
    {
        using DRing::exportable_callback;
        using DRing::DataTransferSignal;

        dataHandlers = {
            exportable_callback<DataTransferSignal::DataConnectionStatus>(
                [this] (const DRing::DataConnectionId& connectionID,
                        int code) {
                        Q_EMIT this->dataConnectionStatusChanged(QString(connectionID.c_str()),
                                                                code);
            }),
            exportable_callback<DataTransferSignal::DataTransferStatus>(
                [this] (const DRing::DataTransferId& transferID,
                        int code) {
                        Q_EMIT this->dataTransferStatusChanged(QString(transferID.c_str()),
                                                                code);
            })
         };
    }

    ~DataTransferManagerInterface() {}

public Q_SLOTS: // METHODS
    QString connectToPeer(const QString& accountId, const QString& peerUri)
    {
        return QString(DRing::connectToPeer(accountId.toStdString(), peerUri.toStdString()).c_str());
    }

    bool dataConnectionInfo(const QString& id, DRing::DataConnectionInfo& info)
    {
        return DRing::dataConnectionInfo(id.toStdString(), info);
    }

    bool closeDataConnection(const QString& id)
    {
        return DRing::closeDataConnection(id.toStdString());
    }

    QString sendFile(const QString& accountId, const QString& peerUri, const QString& filename, const QString& displayname)
    {
        return QString(DRing::sendFile(accountId.toStdString(),
                                        peerUri.toStdString(),
                                        filename.toStdString(),
                                        displayname.toStdString()).c_str());
    }

    bool dataTransferInfo(const QString& id, DRing::DataTransferInfo& info)
    {
        return DRing::dataTransferInfo(id.toStdString(), info);
    }

    std::streamsize dataTransferSentBytes(const QString& id)
    {
        return DRing::dataTransferSentBytes(id.toStdString());
    }

    bool cancelDataTransfer(const QString& id)
    {
        return DRing::cancelDataTransfer(id.toStdString());
    }

    void acceptFileTransfer(const QString& id, const QString& pathname)
    {
        return DRing::acceptFileTransfer(id.toStdString(), pathname.toStdString());
    }


Q_SIGNALS: // SIGNALS
    void dataConnectionStatusChanged(const QString& /*connectionID*/,
                                    int /*code*/);
    void dataTransferStatusChanged(const QString& /*transferID*/,
                                    int /*code*/);
};

namespace org {
  namespace ring {
    namespace Ring {
      typedef ::DataTransferManagerInterface DataTransferManager;
    }
  }
}
