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

#include <string>

namespace lrc
{

constexpr static auto
DATABASE_VERSION="1";
constexpr static auto
DATABASE_STORE_VERSION="PRAGMA schema.user_version=";

constexpr static auto
DATABASE_CREATE_ACCOUNTS_TABLES="CREATE TABLE accounts \
(id integer primary key)";
constexpr static auto
DATABASE_CREATE_CONTACTS_TABLES="CREATE TABLE contacts \
(id integer primary key, unread integer, ring_id text not null unique, \
alias text, photo text, username text, type text)";
constexpr static auto
DATABASE_CREATE_CONVERSATIONS_TABLES="CREATE TABLE conversations \
(id integer primary key, contact integer, account integer, body text, \
timestamp text, is_unread boolean, is_outgoing boolean, type text, status text)";


constexpr static auto
DATABASE_GET_LAST_INSERTED_ID="SELECT last_insert_rowid()";

static const std::string
DATABASE_UPDATE_CONTACT(const std::string& ring_id, const std::string& new_username)
{
    return "UPDATE contacts SET username='" + new_username + "' WHERE ring_id='" + ring_id + "'";
}

static const std::string
DATABASE_GET_CONTACT_ATTRIBUTE(const std::string& ring_id, const std::string& attribute)
{
    return "SELECT " + attribute + " FROM contacts WHERE ring_id='" + ring_id + "'";
}

static const std::string
DATABASE_ADD_CONTACT(const std::string& ring_id,
                     const std::string& username,
                     const std::string& alias,
                     const std::string& photo)
{
    return "INSERT INTO contacts(ring_id, username, alias, photo) values("
    + ring_id + ", " + username + ", " + alias + ", " + photo + ")";
}

static const std::string
DATABASE_SET_MESSAGE_READ(const std::string& message_id)
{
    return "UPDATE conversations SET is_unread='0' WHERE id='"
    + message_id + "'";
}

static const std::string
DATABASE_COUNT_MESSAGES_UNREAD(const std::string& account_id, const std::string& contact_id)
{
    return "SELECT COUNT(is_unread) FROM conversations WHERE is_unread='1' \
    AND contact='" + contact_id + "' AND account='" + account_id + "'";
}

static const std::string
DATABASE_GET_MESSAGES(const std::string& account_id, const std::string& contact_id)
{
    return "SELECT id, contact, body, timestamp, is_outgoing, type, status \
    FROM conversations WHERE contact='" + contact_id
    + "' AND account='" + account_id + "'";
}

static const std::string
DATABASE_REMOVE_HISTORY(const std::string& account_id, const std::string& contact_id)
{
    return "DELETE FROM conversations WHERE contact='" + contact_id
    + "' AND account='" + account_id + "'";
}

static const std::string
DATABASE_ADD_MESSAGE(const std::string& contact_id,
                     const std::string& account_id,
                     const std::string& body,
                     const std::string& timestamp,
                     const bool is_outgoing,
                     const std::string& type,
                     const std::string& status)
{
    return "INSERT INTO conversations(contact, account, body, timestamp, \
    is_unread, is_outgoing, type, status) VALUES('" + contact_id + "', '"
    + account_id + "', '" + body + "', " + timestamp + ", 1, "
    + std::to_string(is_outgoing) + ", '" + type + "', '" + status + "')";
}

} // namespace lrc
