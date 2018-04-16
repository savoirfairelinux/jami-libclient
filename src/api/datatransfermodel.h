/****************************************************************************
 *   Copyright (C) 2018 Savoir-faire Linux                                  *
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

// Std
#include <string>
#include <memory>
#include <ios>

// Qt
#include <qobject.h>

// Data
#include "api/datatransfer.h"
#include "api/account.h"

// LRC
#include "typedefs.h"

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

public:
    DataTransferModel();
    ~DataTransferModel();

    void sendFile(const std::string& account_id, const std::string& peer_uri,
                  const std::string& file_path, const std::string& display_name);

    void transferInfo(long long ringId, datatransfer::Info& lrc_info);

    void bytesProgress(int interactionId, int64_t& total, int64_t& progress);

    void accept(int interactionId, const std::string& file_path, std::size_t offset);

    void cancel(int interactionId);

    void registerTransferId(long long dringId, int interactionId);

    int getInteractionIdFromDringId(long long dringId);

    long long getDringIdFromInteractionId(int interactionId);

    /**
     * Used when images < 20 Mb are automatically accepted and downloaded
     * Should contains the full directory with the end marker (/ on linux for example)
     */
    std::string downloadDirectory;

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
    void transferStatusChanged(const std::string& uid, datatransfer::Status status);

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

}} // namespace lrc::api
