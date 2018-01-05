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

// LRC
#include "typedefs.h"

namespace lrc { namespace api {

namespace datatransfer {

enum class Status {
    on_connection, // outgoing tx: wait for connection/acceptance, incoming tx: wait for local acceptance
    unjoinable_peer, // error: (outgoing only) peer connection failed
    invalid_pathname, // error: (file transfer only) given file is not a valid
    on_progress, // progress reporting
    success, // transfer finished with success, all data sent
    stop_by_peer, // error: transfer terminated by peer
    stop_by_host, // eror: transfer terminated by local host
};

struct Info
{
    std::string uid; ///< long-term and unique identifier (used for historic)
    Status status;
    bool isOutgoing;
    qulonglong totalSize;
    qulonglong progress; ///< if status >= on_progress, gives number of bytes tx/rx until now
    QString path;
    QString displayName;
};

} // namespace lrc::api::datatransfer

}} // namespace lrc::api
