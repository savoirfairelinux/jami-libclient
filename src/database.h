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


// TEST
#include <qdebug.h>

constexpr char ringDB[] = "ring.db";

/// NOTES :
///   - les messages peuvent contenir des images, soit les traiter comme des blobs soit les mettre ailleurs et les
///      charger ultérieurement.

class DataBase : public QObject {
    Q_OBJECT
    public:
    //TODO complete this class and move it elsewhere?
    struct Message
    {
        std::string body;
        std::string timestamp;
    };

    ~DataBase();

    // Messages
    void addMessage(const QString& From, const QString& message, const QString& timestamp);
    std::vector<Message> getMessages(const QString& author);
    int NumberOfUnreads(const QString& author);
    void setMessageRead(const int uid);

    // Contacts
    void addContact(const QString& From, const QByteArray& payload);
    std::string getAlias(const QString& from);
    std::string getAvatar(const QString& from);

    //Singleton
    static DataBase& instance();

    // signals
    Q_SIGNALS:
    void messageAdded(const DataBase::Message);
    void contactAdded(const std::string&);

    private:
    explicit DataBase(QObject* parent = nullptr);

    std::unique_ptr<QSqlQuery> _query;
    QSqlDatabase _db;

};
