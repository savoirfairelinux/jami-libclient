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
#include "databasemanager.h"

// Qt
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

namespace lrc
{

DatabaseManager::DatabaseManager(QObject* parent)
: QObject(parent)
{

}

DatabaseManager::~DatabaseManager()
{

}

void
DatabaseManager::addMessage(const std::string& account, const message::Info& message) const
{
    // TODO improve
    auto addMessageQuery = QString("INSERT INTO conversations(contact, account,\
    body, timestamp, is_unread, is_outgoing, type, status) VALUES(?, ?, ?, ?,\
    1, ?, ?, ?)");

    if (not query_->prepare(addMessageQuery)) {
        qDebug() << "DatabaseManager: addMessage, " << query_->lastError().text();
        return;
    }

    query_->addBindValue(QString(message.uid.c_str()));
    query_->addBindValue(QString(account.c_str()));
    query_->addBindValue(QString(message.body.c_str()));
    query_->addBindValue(QString::number(message.timestamp));
    query_->addBindValue(message.isOutgoing);
    switch (message.type) {
    case message::Type::TEXT:
        query_->addBindValue(QString("TEXT"));
        break;
    case message::Type::CALL:
        query_->addBindValue(QString("CALL"));
        break;
    case message::Type::CONTACT:
        query_->addBindValue(QString("CONTACT"));
        break;
    case message::Type::INVALID_TYPE:
        query_->addBindValue(QString("INVALID_TYPE"));
        break;
    }
    switch (message.status) {
    case message::Status::SENDING:
        query_->addBindValue(QString("SENDING"));
        break;
    case message::Status::FAILED:
        query_->addBindValue(QString("FAILED"));
        break;
    case message::Status::SUCCEED:
        query_->addBindValue(QString("SUCCEED"));
        break;
    case message::Status::INVALID_STATUS:
        query_->addBindValue(QString("INVALID_STATUS"));
        break;
    }

    if (not query_->exec()) {
        qDebug() << "DatabaseManager: addMessage, " << query_->lastError().text();
        return;
    }

    if (not query_->exec("SELECT last_insert_rowid()")) {
        qDebug() << "DatabaseManager: addMessage, " << query_->lastError().text();
        return;
    }
    while(query_->next()) {
        emit messageAdded(query_->value(0).toInt(), account, message);
    }
}

void
DatabaseManager::removeHistory(const std::string& account, const std::string& uid) const
{

}

MessagesMap
DatabaseManager::getMessages(const std::string& account, const std::string& uid) const
{
    return MessagesMap();
}

unsigned int
DatabaseManager::numberOfUnreads(const std::string& account, const std::string& uid) const
{
    return 0;
}

void
DatabaseManager::setMessageRead(int uid)
{

}

void
DatabaseManager::addContact(const std::string& contact, const QByteArray& payload)
{

}

std::string
DatabaseManager::getUri(const std::string& uid) const
{
    return "";
}

std::string
DatabaseManager::getAlias(const std::string& uid) const
{
    return "";
}

std::string
DatabaseManager::getAvatar(const std::string& uid) const
{
    return "";
}

}
