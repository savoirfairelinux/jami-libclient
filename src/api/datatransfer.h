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

// std
#include <ctime>

// LRC
#include "typedefs.h"

namespace lrc { namespace api {

namespace datatransfer {

enum class Status {
    on_connection, // outgoing tx: wait for connection/acceptance, incoming tx: wait for local acceptance
    on_progress, // connected, data transfer progress reporting
    success, // transfer finished with success, all data sent
    stop_by_peer, // error: transfer terminated by peer
    stop_by_host, // eror: transfer terminated by local host
    unjoinable_peer, // error: (outgoing only) peer connection failed
    timeout_expired, // error: (outgoing only) peer awaiting too long, close turn socket
    invalid_pathname, // error: (file transfer only) given file is not a valid
    unsupported, // error: unable to do the transfer (generic error)
    INVALID
};

static inline const std::string
to_string(const Status& status)
{
    switch(status) {
    case Status::on_connection:
        return "on_connection";
    case Status::on_progress:
        return "on_progress";
    case Status::success:
        return "success";
    case Status::stop_by_peer:
        return "stop_by_peer";
    case Status::stop_by_host:
        return "stop_by_host";
    case Status::unjoinable_peer:
        return "unjoinable_peer";
    case Status::timeout_expired:
        return "timeout_expired";
    case Status::invalid_pathname:
        return "invalid_pathname";
    case Status::unsupported:
        return "unsupported";
    case Status::INVALID:
    default:
        return "INVALID";
    }
}

static inline Status
to_status(const std::string& status)
{
    if (status == "on_connection")
        return datatransfer::Status::on_connection;
    else if (status == "on_progress")
        return datatransfer::Status::on_progress;
    else if (status == "success")
        return datatransfer::Status::success;
    else if (status == "stop_by_peer")
        return datatransfer::Status::stop_by_peer;
    else if (status == "stop_by_host")
        return datatransfer::Status::stop_by_host;
    else if (status == "unjoinable_peer")
        return datatransfer::Status::unjoinable_peer;
    else if (status == "timeout_expired")
        return datatransfer::Status::timeout_expired;
    else if (status == "invalid_pathname")
        return datatransfer::Status::invalid_pathname;
    else if (status == "unsupported")
        return datatransfer::Status::unsupported;
    else
        return datatransfer::Status::INVALID;

}

struct Info
{
    std::string uid; ///< long-term and unique identifier (used for historic)
    Status status;
    bool isOutgoing;
    std::size_t totalSize;
    std::size_t progress; ///< if status >= on_progress, gives number of bytes tx/rx until now
    std::string path;
    std::string displayName;
    std::string accountId;
    std::string peerUri;
    std::time_t timestamp = 0;
};

} // namespace lrc::api::datatransfer

}} // namespace lrc::api
