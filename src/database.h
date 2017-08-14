/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
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
#include <string>
#include <memory>
#include <vector>
#include <forward_list>

// Qt
#include <qobject.h>
#include <QtSql/QSqlQuery>

// Lrc
#include "namedirectory.h"

// TEST
#include <qdebug.h>

constexpr char ringDB[] = "ring.db";

/// NOTES :
///   - les messages peuvent contenir des images, soit les traiter comme des blobs soit les mettre ailleurs et les
///      charger ultérieurement.

class Account;

class DataBase : public QObject {
    Q_OBJECT
    public:
    //TODO complete this class and move it elsewhere?
    struct Message
    {
        std::string body;
        std::string timestamp;
        bool is_outgoing;
    };

    ~DataBase();

    // Messages
    void addMessage(const QString& contact, const QString& account, const QString& message, const QString& timestamp, const bool is_outgoing);
    std::vector<Message> getMessages(const QString& contact, const QString& account);
    void removeHistory(const QString& contact, const QString& account);
    int NumberOfUnreads(const QString& contact, const QString& account);
    void setMessageRead(const int uid);

    // Contacts
    void addContact(const QString& From, const QByteArray& payload);
    std::string getUri(const QString& from);
    std::string getAlias(const QString& from);
    std::string getAvatar(const QString& from);

    //Singleton
    static DataBase& instance();

    // signals
    Q_SIGNALS:
    void messageAdded(const DataBase::Message);
    void contactAdded(const std::string&);

    // Slots
    private Q_SLOTS:
    void slotRegisteredNameFound(const Account* account, NameDirectory::LookupStatus status, const QString& address, const QString& name);

    private:
    explicit DataBase(QObject* parent = nullptr);

    std::unique_ptr<QSqlQuery> _query;
    QSqlDatabase _db;

};
