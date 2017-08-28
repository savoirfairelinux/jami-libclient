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

// Std
#include <memory>
#include <string>

// Qt
#include <qobject.h>
#include <QtSql/QSqlQuery>

// Data
#include "data/message.h"

// Lrc
#include "typedefs.h"
#include "namedirectory.h"

constexpr char ringDB[] = "ring.db";

class Account;
class Lrc;

namespace lrc
{

class LIB_EXPORT Database : public QObject {
    Q_OBJECT

    friend class Lrc;

public:
    ~Database();

    // Messages related
    /**
     * Add a message object into the database
     * @param accountId
     * @param message to add
     */
    void addMessage(const std::string& accountId, const message::Info& message) const;
    /**
     * Clear the history of the conversation between account and uid
     * @param accountId
     * @param uid
     * @param removeContact if we also want to remove the contact
     */
    void clearHistory(const std::string& accountId,
                      const std::string& uid,
                      bool removeContact = false) const;
    /**
     * @param  accountId
     * @param  uid
     * @return history of the conversation between account and uid
     */
    MessagesMap getHistory(const std::string& accountId, const std::string& uid) const;
    /**
     * @param  accountId
     * @param  uid
     * @return number of unread messages in the conversation between account and uid
     */
    std::size_t numberOfUnreads(const std::string& accountId, const std::string& uid) const;
    /**
     * Set a message READ
     * @param uid of the message to update
     */
    void setMessageRead(int uid) const;

    // Contacts related
    /**
     * Add a contact into the database
     * @param contact
     * @param payload the VCard of this contact
     */
    void addContact(const std::string& contact, const QByteArray& payload) const;
    /**
     * @param  uid
     * @param  attribute to search (correpond to a column of the "contacts" tables)
     * @return attribute of a contact
     */
    std::string getContactAttribute(const std::string& contact, const std::string& attribute) const;

Q_SIGNALS:
    /**
     * Will be emitted each time a message is successfully stored into the database
     * @param uid of the message
     * @param accountId linked to the conversation
     * @param message added
     */
    void messageAdded(int uid, const std::string& accountId, message::Info msg) const;
    /**
     * Will be emitted each time a contact is added into the database
     * @param uid of the contact
     */
    void contactAdded(const std::string& uid) const;

private Q_SLOTS:
    void slotRegisteredNameFound(const Account* account,
                                 NameDirectory::LookupStatus status,
                                 const QString& address,
                                 const QString& name) const;

private:
    explicit Database();
    std::unique_ptr<QSqlQuery> query_;
    QSqlDatabase db_;

};

} // namespace lrc
