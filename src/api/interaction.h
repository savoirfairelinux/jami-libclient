/****************************************************************************
 *    Copyright (C) 2017-2021 Savoir-faire Linux Inc.                                  *
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

#include <QString>
#include <QObject>

#include <ctime>
#include "typedefs.h"

namespace lrc {

namespace api {

namespace interaction {
Q_NAMESPACE
Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

enum class Type { INVALID, TEXT, CALL, CONTACT, DATA_TRANSFER, MERGE, COUNT__ };
Q_ENUM_NS(Type)

static inline const QString
to_string(const Type& type)
{
    switch (type) {
    case Type::TEXT:
        return "TEXT";
    case Type::CALL:
        return "CALL";
    case Type::CONTACT:
        return "CONTACT";
    case Type::DATA_TRANSFER:
        return "DATA_TRANSFER";
    case Type::MERGE:
        return "MERGE";
    case Type::INVALID:
    case Type::COUNT__:
    default:
        return "INVALID";
    }
}

static inline Type
to_type(const QString& type)
{
    if (type == "TEXT" || type == "text/plain")
        return interaction::Type::TEXT;
    else if (type == "CALL" || type == "application/call-history+json")
        return interaction::Type::CALL;
    else if (type == "CONTACT" || type == "member")
        return interaction::Type::CONTACT;
    else if (type == "DATA_TRANSFER" || type == "application/data-transfer+json")
        return interaction::Type::DATA_TRANSFER;
    else if (type == "merge")
        return interaction::Type::MERGE;
    else
        return interaction::Type::INVALID;
}

enum class Status {
    INVALID,
    UNKNOWN,
    SENDING,
    FAILURE,
    SUCCESS,
    DISPLAYED,
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
Q_ENUM_NS(Status)

static inline const QString
to_string(const Status& status)
{
    switch (status) {
    case Status::UNKNOWN:
        return "UNKNOWN";
    case Status::SENDING:
        return "SENDING";
    case Status::FAILURE:
        return "FAILURE";
    case Status::SUCCESS:
        return "SUCCESS";
    case Status::DISPLAYED:
        return "DISPLAYED";
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
    case Status::COUNT__:
    default:
        return "INVALID";
    }
}

static inline Status
to_status(const QString& status)
{
    if (status == "UNKNOWN")
        return Status::UNKNOWN;
    else if (status == "SENDING")
        return Status::SENDING;
    else if (status == "FAILURE")
        return Status::FAILURE;
    else if (status == "SUCCESS")
        return Status::SUCCESS;
    else if (status == "DISPLAYED")
        return Status::DISPLAYED;
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

enum class ContactAction { ADD, JOIN, LEAVE, BANNED, INVALID };
Q_ENUM_NS(ContactAction)

static inline const QString
to_string(const ContactAction& action)
{
    switch (action) {
    case ContactAction::ADD:
        return "ADD";
    case ContactAction::JOIN:
        return "JOIN";
    case ContactAction::LEAVE:
        return "LEAVE";
    case ContactAction::BANNED:
        return "BANNED";
    case ContactAction::INVALID:
        return {};
    }
    return {};
}

static inline ContactAction
to_action(const QString& action)
{
    if (action == "add")
        return ContactAction::ADD;
    else if (action == "join")
        return ContactAction::JOIN;
    else if (action == "remove")
        return ContactAction::LEAVE;
    else if (action == "banned")
        return ContactAction::BANNED;
    return ContactAction::INVALID;
}

static inline QString
getContactInteractionString(const QString& authorUri, const ContactAction& action)
{
    switch (action) {
    case ContactAction::ADD:
        if (authorUri.isEmpty()) {
            return QObject::tr("Contact added");
        }
        return QObject::tr("Invitation received");
    case ContactAction::JOIN:
        return QObject::tr("Invitation accepted");
    case ContactAction::LEAVE:
        return QObject::tr("Contact left conversation");
    case ContactAction::BANNED:
    case ContactAction::INVALID:
        return {};
    }
    return {};
}

/**
 * @var authorUri
 * @var body
 * @var timestamp
 * @var duration
 * @var type
 * @var status
 * @var isRead
 * @var commit
 * @var linkPreviewInfo
 * @var linkified
 */
struct Info
{
    QString authorUri;
    QString body;
    QString parentId = "";
    std::time_t timestamp = 0;
    std::time_t duration = 0;
    Type type = Type::INVALID;
    Status status = Status::INVALID;
    bool isRead = false;
    MapStringString commit;
    QVariantMap linkPreviewInfo = {};
    bool linkified = false;
    QVariantMap transferStats = {};

    Info() {}

    Info(QString authorUri,
         QString body,
         std::time_t timestamp,
         std::time_t duration,
         Type type,
         Status status,
         bool isRead)
    {
        this->authorUri = authorUri;
        this->body = body;
        this->timestamp = timestamp;
        this->duration = duration;
        this->type = type;
        this->status = status;
        this->isRead = isRead;
    }

    Info(const MapStringString& message, const QString& accountURI)
    {
        type = to_type(message["type"]);
        if (type == Type::TEXT) {
            body = message["body"];
        }
        authorUri = accountURI == message["author"] ? "" : message["author"];
        timestamp = message["timestamp"].toInt();
        status = Status::SUCCESS;
        parentId = message["linearizedParent"];
        isRead = false;
        if (type == Type::CONTACT) {
            authorUri = accountURI == message["uri"] ? "" : message["uri"];
            body = getContactInteractionString(authorUri, to_action(message["action"]));
        }
        if (message["type"] == "initial" && message.find("invited") != message.end()) {
            type = Type::CONTACT;
            authorUri = accountURI != message["invited"] ? "" : message["invited"];
            body = getContactInteractionString(authorUri, ContactAction::ADD);
        }
        if (type == Type::CALL) {
            duration = message["duration"].toInt() / 1000;
        }
        commit = message;
    }
};

static inline bool
isOutgoing(const Info& interaction)
{
    return interaction.authorUri.isEmpty();
}

} // namespace interaction
} // namespace api
} // namespace lrc
