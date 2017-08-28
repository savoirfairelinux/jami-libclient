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
#include "database.h"

// Qt
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

// LRC
#include "private/vcardutils.h"
#include "availableaccountmodel.h"

namespace lrc
{

Database::Database()
: QObject()
{

}

Database::~Database()
{

}

void
Database::addMessage(const std::string& accountId, const message::Info& message) const
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
    query_->addBindValue(QString(accountId.c_str()));
    query_->addBindValue(QString(message.body.c_str()));
    query_->addBindValue(QString::number(message.timestamp));
    query_->addBindValue(false); // TODO
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
    case message::Type::INVALID:
        query_->addBindValue(QString("INVALID"));
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
    case message::Status::INVALID:
        query_->addBindValue(QString("INVALID"));
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
        emit messageAdded(query_->value(0).toInt(), accountId, message);
    }
}

void
Database::clearHistory(const std::string& accountId, const std::string& uid, bool removeContact) const
{

}

MessagesMap
Database::getAllMessages(const std::string& accountId, const std::string& uid) const
{

}

std::size_t
Database::numberOfUnreads(const std::string& accountId, const std::string& uid) const
{

}

void
Database::setMessageRead(int uid) const
{

}

void
Database::addContact(const std::string& contact, const QByteArray& payload) const
{

}

std::string
Database::getContactAttribute(const std::string& uid, const std::string& attribute) const
{

}

} // namespace lrc
