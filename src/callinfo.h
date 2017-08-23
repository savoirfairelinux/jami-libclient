/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
 *   Author : Sébastien Blin <sebastien.blin@savoirfairelinux.com>          *
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
#include "account.h"

namespace NewCall
{

enum class Status {
    NONE,
    OUTGOING_REQUESTED,
    INCOMING_RINGING,
    OUTGOING_RINGING,
    CONNECTING,
    SEARCHING,
    IN_PROGRESS,
    PAUSED,
    PEER_PAUSED,
    INACTIVE,
    ENDED,
    TERMINATING,
    CONNECTED,
    AUTO_ANSWERING,
    INVALID_STATUS
};

struct Info
{
    const std::string id_;
    const std::time_t startTime_;
    Status status_;
    Account* account_;
    Info(Account* account, const std::string& id, std::time_t startTime, Status status)
    : account_(account), id_(id), startTime_(startTime), status_(status) {};
};

}

typedef std::map<std::string, std::shared_ptr<NewCall::Info>> CallsInfo;
