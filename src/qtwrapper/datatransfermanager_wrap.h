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
                [this] (const std::string& accountId,
                        const std::string& id,
                        DRing::DataConnectionStatus status,
                        DRing::DataTransferError error) {
                       QTimer::singleShot(0, [this, accountId, id, status, error] {
                             Q_EMIT this->dataConnectionStatusChanged(QString(accountId.c_str()),
                                                                    QString(id.c_str()),
                                                                    status,
                                                                    error);
                       });
            }),
            exportable_callback<DataTransferSignal::FileTransferStatus>(
                [this] (const std::string& accountId,
                        const std::string& id,
                        DRing::FileTransferStatus status,
                        DRing::DataTransferError error) {
                       QTimer::singleShot(0, [this,accountId, id, status, error] {
                             Q_EMIT this->fileTransferStatusChanged(QString(accountId.c_str()),
                                                                    QString(id.c_str()),
                                                                    status,
                                                                    error);
                       });
            })
         };
    }

    ~DataTransferManagerInterface() {}

public Q_SLOTS: // METHODS
    QString sendFile(const QString& accountId, const QString& peerUri, const QString& filename)
    {
        return QString(DRing::sendFile(accountId.toStdString(), peerUri.toStdString(), filename.toStdString()).c_str());
    }

    bool fileTransferInfo(const QString& id, DRing::FileTransferInfo& info)
    {
        return DRing::fileTransferInfo(id.toStdString(), info);
    }

    std::size_t fileTransferProgress(const QString& id)
    {
        return DRing::fileTransferProgress(id.toStdString());
    }

    bool cancelFileTransfer(const QString& id)
    {
        return DRing::cancelFileTransfer(id.toStdString());
    }

Q_SIGNALS: // SIGNALS
    void dataConnectionStatusChanged(const QString& /*accountID*/,
                                    const QString& /*connectionID*/,
                                    DRing::DataConnectionStatus /*status*/,
                                    DRing::DataTransferError /*error*/);
    void fileTransferStatusChanged(const QString& /*accountID*/,
                                    const QString& /*connectionID*/,
                                    DRing::FileTransferStatus /*status*/,
                                    DRing::DataTransferError /*error*/);
};

namespace org {
  namespace ring {
    namespace Ring {
      typedef ::DataTransferManagerInterface DataTransferManager;
    }
  }
}
