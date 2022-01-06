/****************************************************************************
 *    Copyright (C) 2018-2022 Savoir-faire Linux Inc.                       *
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
class LIB_EXPORT DataTransferModel : public QObject
{
    Q_OBJECT
public:
    DataTransferModel();
    ~DataTransferModel();

    void sendFile(const QString& account_id,
                  const QString& peer_uri,
                  const QString& conversationId,
                  const QString& file_path,
                  const QString& display_name);

    void transferInfo(const QString& accountId, const QString& fileId, datatransfer::Info& lrc_info);

    void fileTransferInfo(const QString& accountId,
                          const QString& conversationId,
                          const QString& fileId,
                          QString& path,
                          qlonglong& total,
                          qlonglong& progress);

    QString accept(const QString& accountId, const QString& fileId, const QString& filePath = {});

    void download(const QString& accountId,
                  const QString& convId,
                  const QString& interactionId,
                  const QString& fileId,
                  const QString& filePath = {});

    void copyTo(const QString& accountId,
                const QString& convId,
                const QString& interactionId,
                const QString& destPath,
                const QString& displayName);

    void cancel(const QString& accountId, const QString& conversationId, const QString& fileId);

    void registerTransferId(const QString& fileId, const QString& interactionId);

    QString getInteractionIdFromFileId(const QString& fileId);

    QString getFileIdFromInteractionId(const QString& fileId);

    /**
     *  Creates APPDATA/received and return the path
     */
    static QString createDefaultDirectory();

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
} // namespace api
} // namespace lrc
Q_DECLARE_METATYPE(lrc::api::DataTransferModel*)
