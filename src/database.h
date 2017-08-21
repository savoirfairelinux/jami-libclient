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

// Lrc
#include "data/message.h"
#include "typedefs.h"
#include "namedirectory.h"
#include "lrc.h"

constexpr char ringDB[] = "ring.db";

class Account;

namespace lrc
{

class LIB_EXPORT Database : public QObject {
    Q_OBJECT

    friend Lrc::Lrc();

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
     * @return messages from the conversation between account and uid
     */
    MessagesMap getAllMessages(const std::string& accountId, const std::string& uid) const;
    /**
     * @param  accountId
     * @param  uid
     * @return the number of unread messages in the conversation between account and uid
     */
    std::size_t numberOfUnreads(const std::string& accountId, const std::string& uid) const;
    /**
     * Set a message READ
     * @param uid of a message
     */
    void setMessageRead(int uid) const;

    // Contacts related
    /**
     * Add a contact into the database
     * @param contact a uid
     * @param payload the VCard of a contact
     */
    void addContact(const std::string& contact, const QByteArray& payload) const;
    /**
     * @param  uid of a contact
     * @param  the attribute to search for a contact
     * @return attribute of a contact
     */
    std::string getContactAttribute(const std::string& uid, const std::string& attribute) const;

Q_SIGNALS:
    /**
     * Will be emitted each time a message is successfully stored into the database
     * @param uid the uid of the message
     * @param accountId linked to the conversation
     * @param msg the message added
     */
    void messageAdded(int uid, const std::string& accountId, message::Info msg) const;
    /**
     * Will be emitted each time a contact is added into the database
     * @param uid the uid of the contact
     */
    void contactAdded(const std::string& uid) const;

private:
    explicit Database();
    std::unique_ptr<QSqlQuery> query_;
    QSqlDatabase db_;

};

} // namespace lrc
