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
#include "message.h"
#include "typedefs.h"

constexpr char ringDB[] = "ring.db";

class Account;

namespace lrc
{

class LIB_EXPORT DatabaseManager : public QObject {
    Q_OBJECT
public:
    explicit DatabaseManager(QObject* parent = nullptr);
    ~DatabaseManager();

    // Messages related
    /**
     * Add a message object into the database
     * @param account linked to message
     * @param message the object to add
     */
    void addMessage(const std::string& account, const message::Info& message) const;
    void removeHistory(const std::string& account, const std::string& uid) const;
    MessagesMap getMessages(const std::string& account, const std::string& uid) const;
    unsigned int numberOfUnreads(const std::string& account, const std::string& uid) const;
    void setMessageRead(int uid);

    // Contacts
    void addContact(const std::string& contact, const QByteArray& payload);
    std::string getUri(const std::string& uid) const;
    std::string getAlias(const std::string& uid) const;
    std::string getAvatar(const std::string& uid) const;

    // signals
Q_SIGNALS:
    /**
     * Will be emitted each time a message is successfully stored into the database
     * @param uid the uid of the message
     * @param account linked to the conversation
     * @param msg the message added
     */
    void messageAdded(int uid, const std::string& account, message::Info msg) const;
    void contactAdded(const std::string&);

private:
    std::unique_ptr<QSqlQuery> query_;
    QSqlDatabase db_;

};

}
