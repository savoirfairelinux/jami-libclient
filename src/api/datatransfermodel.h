/****************************************************************************
 *    Copyright (C) 2018-2020 Savoir-faire Linux Inc.                                  *
 *   Author: Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>       *
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

#include "api/datatransfer.h"
#include "api/account.h"

#include "typedefs.h"

#include <QObject>

#include <string>
#include <memory>
#include <ios>

namespace lrc {

class CallbacksHandler;
class Database;

namespace api {

class BehaviorController;

namespace datatransfer {
class Info;
} // namespace datatransfer

/**
 *  @brief Class that manages data transfer.
 */
class LIB_EXPORT DataTransferModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString downloadDirectory_qml MEMBER downloadDirectory)
public:
    DataTransferModel();
    ~DataTransferModel();

    Q_INVOKABLE void sendFile(const QString& account_id, const QString& peer_uri,
        const QString& file_path, const QString& display_name);

    Q_INVOKABLE void transferInfo(long long ringId, datatransfer::Info& lrc_info);

    Q_INVOKABLE void bytesProgress(int interactionId, int64_t& total, int64_t& progress);

    Q_INVOKABLE QString accept(int interactionId, const QString& file_path, std::size_t offset);

    Q_INVOKABLE void cancel(int interactionId);

    Q_INVOKABLE void registerTransferId(long long dringId, int interactionId);

    Q_INVOKABLE int getInteractionIdFromDringId(long long dringId);

    Q_INVOKABLE long long getDringIdFromInteractionId(int interactionId);

    /**
     * Used when images < 20 Mb are automatically accepted and downloaded
     * Should contains the full directory with the end marker (/ on linux for example)
     */
    QString downloadDirectory;

    /**
     *  Creates APPDATA/received and return the path
     */
    Q_INVOKABLE static QString createDefaultDirectory();

    /**
     * Accept transfer from untrusted contacts
     */
    bool acceptFromUnstrusted{ false };

    /**
     * Accept transfer from trusted contacts
     */
    bool automaticAcceptTransfer{ true };

    /**
     * Automatically accept transfer under
     */
    unsigned acceptBehindMb{ 20 } /* Mb */;

Q_SIGNALS:
    /**
     * Connect this signal to know when a data transfer is incoming.
     */
    void incomingTransfer(api::datatransfer::Info dataTransferInfo);

    /**
     * Connect this signal to know when an existing data transfer has changed of status.
     * @param transfer_id unique identification of incoming data transfer.
     * @param status reported status.
     */
    void transferStatusChanged(const QString& uid, datatransfer::Status status);

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};
}
} // namespace lrc::api
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
Q_DECLARE_METATYPE(lrc::api::DataTransferModel*)
#endif
