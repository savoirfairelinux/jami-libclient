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

class LIB_EXPORT DatabaseManager : public QObject {
    Q_OBJECT
    public:
    explicit DatabaseManager(QObject* parent = nullptr);
    ~DatabaseManager();

    // Messages
    void addMessage(const std::string& account, const std::string& uid, const std::string& body, const long timestamp, const bool isOutgoing);
    void removeHistory(const std::string& account, const std::string& uid);
    Messages getMessages(const std::string& account, const std::string& uid) const;
    unsigned int numberOfUnreads(const std::string& account, const std::string& uid) const;
    void setMessageRead(int uid);

    // Contacts
    void addContact(const std::string& contact, const QByteArray& payload);
    std::string getUri(const std::string& uid) const;
    std::string getAlias(const std::string& uid) const;
    std::string getAvatar(const std::string& uid) const;

    // signals
    Q_SIGNALS:
    void messageAdded(const std::string&, const std::string&, Message::Info);
    void contactAdded(const std::string&);

    private:
    std::unique_ptr<QSqlQuery> _query;
    QSqlDatabase _db;

};
