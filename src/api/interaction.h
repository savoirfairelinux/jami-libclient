/****************************************************************************
 *    Copyright (C) 2017-2019 Savoir-faire Linux Inc.                                  *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
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
#include <ctime>
#include <string>

namespace lrc
{

namespace api
{

namespace interaction
{

enum class Type {
    INVALID,
    TEXT,
    CALL,
    CONTACT,
    DATA_TRANSFER,
    COUNT__
};

static inline const std::string
to_string(const Type& type)
{
    switch(type) {
    case Type::TEXT:
        return "TEXT";
    case Type::CALL:
        return "CALL";
    case Type::CONTACT:
        return "CONTACT";
    case Type::DATA_TRANSFER:
        return "DATA_TRANSFER";
    case Type::INVALID:
    default:
        return "INVALID";
    }
}

static inline Type
to_type(const std::string& type)
{
    if (type == "TEXT")
        return interaction::Type::TEXT;
    else if (type == "CALL")
        return interaction::Type::CALL;
    else if (type == "CONTACT")
        return interaction::Type::CONTACT;
    else if (type == "DATA_TRANSFER")
        return interaction::Type::DATA_TRANSFER;
    else
        return interaction::Type::INVALID;
}

enum class Status {
    INVALID,
    UNKNOWN,
    SENDING,
    FAILURE,
    SUCCESS,
    TRANSFER_CREATED,
    TRANSFER_ACCEPTED,
    TRANSFER_CANCELED,
    TRANSFER_ERROR,
    TRANSFER_UNJOINABLE_PEER,
    TRANSFER_ONGOING,
    TRANSFER_AWAITING_PEER,
    TRANSFER_AWAITING_HOST,
    TRANSFER_TIMEOUT_EXPIRED,
    TRANSFER_FINISHED,
    COUNT__
};

static inline const std::string
to_string(const Status& status)
{
    switch(status) {
    case Status::UNKNOWN:
        return "UNKNOWN";
    case Status::SENDING:
        return "SENDING";
    case Status::FAILURE:
        return "FAILURE";
    case Status::SUCCESS:
        return "SUCCESS";
    case Status::TRANSFER_CREATED:
        return "TRANSFER_CREATED";
    case Status::TRANSFER_ACCEPTED:
        return "TRANSFER_ACCEPTED";
    case Status::TRANSFER_CANCELED:
        return "TRANSFER_CANCELED";
    case Status::TRANSFER_ERROR:
        return "TRANSFER_ERROR";
    case Status::TRANSFER_UNJOINABLE_PEER:
        return "TRANSFER_UNJOINABLE_PEER";
    case Status::TRANSFER_ONGOING:
        return "TRANSFER_ONGOING";
    case Status::TRANSFER_AWAITING_HOST:
        return "TRANSFER_AWAITING_HOST";
    case Status::TRANSFER_AWAITING_PEER:
        return "TRANSFER_AWAITING_PEER";
    case Status::TRANSFER_TIMEOUT_EXPIRED:
        return "TRANSFER_TIMEOUT_EXPIRED";
    case Status::TRANSFER_FINISHED:
        return "TRANSFER_FINISHED";
    case Status::INVALID:
    default:
        return "INVALID";
    }
}

static inline Status
to_status(const std::string& status)
{
    if (status == "UNKNOWN")
        return Status::UNKNOWN;
    else if (status == "SENDING")
        return Status::SENDING;
    else if (status == "FAILURE")
        return Status::FAILURE;
    else if (status == "SUCCESS")
        return Status::SUCCESS;
    else if (status == "TRANSFER_CREATED")
        return Status::TRANSFER_CREATED;
    else if (status == "TRANSFER_ACCEPTED")
        return Status::TRANSFER_ACCEPTED;
    else if (status == "TRANSFER_CANCELED")
        return Status::TRANSFER_CANCELED;
    else if (status == "TRANSFER_ERROR")
        return Status::TRANSFER_ERROR;
    else if (status == "TRANSFER_UNJOINABLE_PEER")
        return Status::TRANSFER_UNJOINABLE_PEER;
    else if (status == "TRANSFER_ONGOING")
        return Status::TRANSFER_ONGOING;
    else if (status == "TRANSFER_AWAITING_HOST")
        return Status::TRANSFER_AWAITING_HOST;
    else if (status == "TRANSFER_AWAITING_PEER")
        return Status::TRANSFER_AWAITING_PEER;
    else if (status == "TRANSFER_TIMEOUT_EXPIRED")
        return Status::TRANSFER_TIMEOUT_EXPIRED;
    else if (status == "TRANSFER_FINISHED")
        return Status::TRANSFER_FINISHED;
    else
        return Status::INVALID;

}

/**
 * @var authorUri
 * @var body
 * @var timestamp
 * @var duration
 * @var type
 * @var status
 * @var isRead
 */
struct Info
{
    std::string authorUri;
    std::string body;
    std::time_t timestamp = 0;
    std::time_t duration = 0;
    Type type = Type::INVALID;
    Status status = Status::INVALID;
    bool isRead = false;
};

static inline bool isOutgoing(const Info& interaction) {
    return interaction.authorUri.empty();
}

} // namespace interaction
} // namespace api
} // namespace lrc
