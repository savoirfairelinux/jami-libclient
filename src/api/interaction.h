/****************************************************************************
 *   Copyright (C) 2017-2018 Savoir-faire Linux                                  *
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
    OUTGOING_DATA_TRANSFER,
    INCOMING_DATA_TRANSFER
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
    case Type::OUTGOING_DATA_TRANSFER:
        return "OUTGOING_DATA_TRANSFER";
    case Type::INCOMING_DATA_TRANSFER:
        return "INCOMING_DATA_TRANSFER";
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
    else if (type == "OUTGOING_DATA_TRANSFER")
        return interaction::Type::OUTGOING_DATA_TRANSFER;
    else if (type == "INCOMING_DATA_TRANSFER")
        return interaction::Type::INCOMING_DATA_TRANSFER;
    else
        return interaction::Type::INVALID;
}


enum class Status {
    INVALID,
    UNKNOWN,
    SENDING,
    FAILED,
    SUCCEED,
    READ,
    UNREAD,
    TRANSFER_CREATED, /*[jn] mettre à jour les fonctions de conversion */
    TRANSFER_ACCEPTED,
    TRANSFER_CANCELED,
    TRANSFER_ERROR,
    TRANSFER_ONGOING,
    TRANSFER_AWAITING,
    TRANSFER_FINISHED
};

static inline const std::string
to_string(const Status& status)
{
    switch(status) {
    case Status::UNKNOWN:
        return "UNKNOWN";
    case Status::SENDING:
        return "SENDING";
    case Status::FAILED:
        return "FAILED";
    case Status::SUCCEED:
        return "SUCCEED";
    case Status::READ:
        return "READ";
    case Status::UNREAD:
        return "UNREAD";
    case Status::TRANSFER_CREATED:
        return "TRANSFER_CREATED";
    case Status::TRANSFER_ACCEPTED:
        return "TRANSFER_ACCEPTED";
    case Status::TRANSFER_CANCELED:
        return "TRANSFER_CANCELED";
    case Status::TRANSFER_ERROR:
        return "TRANSFER_ERROR";
    case Status::TRANSFER_ONGOING:
        return "TRANSFER_ONGOING";
    case Status::TRANSFER_AWAITING:
        return "TRANSFER_AWAITING";
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
        return interaction::Status::UNKNOWN;
    else if (status == "SENDING")
        return interaction::Status::SENDING;
    else if (status == "FAILED")
        return interaction::Status::FAILED;
    else if (status == "SUCCEED")
        return interaction::Status::SUCCEED;
    else if (status == "READ")
        return interaction::Status::READ;
    else if (status == "UNREAD")
        return interaction::Status::UNREAD;
    else if (status == "TRANSFER_CREATED")
        return interaction::Status::TRANSFER_CREATED;
    else if (status == "TRANSFER_ACCEPTED")
        return interaction::Status::TRANSFER_ACCEPTED;
    else if (status == "TRANSFER_CANCELED")
        return interaction::Status::TRANSFER_CANCELED;
    else if (status == "TRANSFER_ERROR")
        return interaction::Status::TRANSFER_ERROR;
    else if (status == "TRANSFER_ONGOING")
        return interaction::Status::TRANSFER_ONGOING;
    else if (status == "TRANSFER_AWAITING")
        return interaction::Status::TRANSFER_AWAITING;
    else if (status == "TRANSFER_FINISHED")
        return interaction::Status::TRANSFER_FINISHED;
    else
        return interaction::Status::INVALID;

}

struct Info
{
    std::string authorUri;
    std::string body;
    std::time_t timestamp = 0;
    Type type = Type::INVALID;
    Status status = Status::INVALID;
};

static inline bool isOutgoing(const Info& interaction) {
    return (interaction.status != lrc::api::interaction::Status::READ
    && interaction.status != lrc::api::interaction::Status::UNREAD)
    || interaction.type == lrc::api::interaction::Type::OUTGOING_DATA_TRANSFER;
}

} // namespace interaction
} // namespace api
} // namespace lrc
