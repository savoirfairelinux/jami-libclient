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

// std
#include <map>

// Qt
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

// Lrc
#include "private/vcardutils.h"
#include "availableaccountmodel.h"

DatabaseManager::DatabaseManager(QObject* parent)
: QObject(parent)
{
    if (not QSqlDatabase::drivers().contains("QSQLITE")) {
        qDebug() << "DatabaseManager, errror QSQLITE not supported";
        return;
    }

    // initalize the database
    db_ = QSqlDatabase::addDatabase("QSQLITE");
    db_.setDatabaseName(ringDB);

    if (not db_.open()) {
        qDebug() << "DatabaseManager, can't open the database";
        return;
    }

    // connect query object to database object
    // You must load the SQL driver and open the connection before a QSqlQuery is created.
    query_ = std::unique_ptr<QSqlQuery>(new QSqlQuery(db_));

    // check if all tables are presents :
    QStringList tables = db_.tables();

    // Set the version of the database
    if (tables.empty())
        if (not query_->exec("PRAGMA schema.user_version = 1"))
            qDebug() << "DatabaseManager: " << query_->lastError().text();

    // add accounts table
    if (not tables.contains("accounts", Qt::CaseInsensitive))
        if (not query_->exec("CREATE TABLE accounts (id integer primary key)"))
            qDebug() << "DatabaseManager: " << query_->lastError().text();

    // add contacts table
    if (not tables.contains("contacts", Qt::CaseInsensitive))
        if (not query_->exec("CREATE TABLE contacts (id integer primary key, \
        unread integer, ring_id text not null unique, alias text, photo text, \
        username text, type text)"))
            qDebug() << "DatabaseManager: " << query_->lastError().text();

    // add conversations table
    if (not tables.contains("conversations", Qt::CaseInsensitive))
        if (not query_->exec("CREATE TABLE conversations (id integer primary key, \
        contact integer, account integer, body text, timestamp text, \
        is_unread boolean, is_outgoing boolean, type text, status text)"))
            qDebug() << "DatabaseManager: " << query_->lastError().text();
}

DatabaseManager::~DatabaseManager()
{

}

#include <iostream>

void
DatabaseManager::addMessage(const std::string& account, const lrc::message::Info& message)
{
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
    case lrc::message::Type::TEXT:
        query_->addBindValue(QString("TEXT"));
        break;
    case lrc::message::Type::CALL:
        query_->addBindValue(QString("CALL"));
        break;
    case lrc::message::Type::CONTACT:
        query_->addBindValue(QString("CONTACT"));
        break;
    case lrc::message::Type::INVALID_TYPE:
        query_->addBindValue(QString("INVALID_TYPE"));
        break;
    }
    switch (message.status) {
    case lrc::message::Status::SENDING:
        query_->addBindValue(QString("SENDING"));
        break;
    case lrc::message::Status::FAILED:
        query_->addBindValue(QString("FAILED"));
        break;
    case lrc::message::Status::SUCCEED:
        query_->addBindValue(QString("SUCCEED"));
        break;
    case lrc::message::Status::INVALID_STATUS:
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
DatabaseManager::removeHistory(const std::string& account, const std::string& uid, bool removeContact)
{
    auto removeHistoryQuery = "DELETE FROM conversations WHERE contact = '"
    +uid+"' AND account='"+account+"'";
    if (!removeContact) {
        removeHistoryQuery += " AND type!='CONTACT'";
    }

    if (not query_->exec(removeHistoryQuery.c_str())) {
        qDebug() << "DatabaseManager: removeHistory, " << query_->lastError().text();
    }
}

lrc::MessagesMap
DatabaseManager::getMessages(const std::string& account, const std::string& uid) const
{
    auto getMessagesQuery = "SELECT id, contact, body, timestamp, \
    is_outgoing, type, status FROM conversations WHERE contact = '" + uid + "' \
    AND account='" + account + "'";

    if (not query_->exec(getMessagesQuery.c_str())) {
        qDebug() << "DatabaseManager: getMessages, " << query_->lastError().text();
        return lrc::MessagesMap();
    }

    lrc::MessagesMap messages;
    while(query_->next()) {
        auto message_id = query_->value(0).toInt();
        lrc::message::Info msg;
        msg.uid = query_->value(1).toString().toStdString();
        msg.body = query_->value(2).toString().toStdString();
        msg.timestamp = std::stoll(query_->value(3).toString().toStdString());
        msg.isOutgoing = query_->value(4).toBool();
        auto typeStr = query_->value(5).toString().toStdString();
        auto type = lrc::message::Type::INVALID_TYPE;
        if (typeStr == "TEXT") {
            type = lrc::message::Type::TEXT;
        } else if (typeStr == "CALL") {
            type = lrc::message::Type::CALL;
        } else if (typeStr == "CONTACT") {
            type = lrc::message::Type::CONTACT;
        }
        auto statusStr = query_->value(5).toString().toStdString();
        auto status = lrc::message::Status::INVALID_STATUS;
        if (statusStr == "SENDING") {
            status = lrc::message::Status::SENDING;
        } else if (statusStr == "FAILED") {
            status = lrc::message::Status::FAILED;
        } else if (statusStr == "SUCCEED") {
            status = lrc::message::Status::SUCCEED;
        }
        msg.type = type;
        msg.status = status;
        messages.insert(std::pair<int, lrc::message::Info>(message_id, msg));
    }

    return messages;
}

unsigned int
DatabaseManager::numberOfUnreads(const std::string& account, const std::string& uid) const
{
    auto numberOfUnreadsQuery = "SELECT COUNT(is_unread) FROM conversations \
    WHERE is_unread='1' AND contact='" + uid + "' AND account='" + account + "'";

    if (not query_->exec(numberOfUnreadsQuery.c_str())) {
        qDebug() << "DatabaseManager: NumberOfUnreads, " << query_->lastError().text();
        return -1;
    }

    query_->next();
    return query_->value(0).toUInt();
}

void
DatabaseManager::setMessageRead(int uid)
{
    auto setMessageReadQuery = QString("UPDATE conversations SET is_unread = '0' \
    WHERE id = '" + QString::number(uid) + "'");

    if (not query_->exec(setMessageReadQuery))
        qDebug() << "DatabaseManager: setMessageRead, " << query_->lastError().text();
}

void
DatabaseManager::addContact(const std::string& contact, const QByteArray& payload)
{
    // NOTE: this function will be improved
    auto contact_id = QString(contact.c_str());
    auto vCard = VCardUtils::toHashMap(payload);

    auto alias = vCard["FN"];
    auto photo = vCard["PHOTO;ENCODING=BASE64;TYPE=PNG"]; // TODO: to improve

    auto addContactQuery = QString("INSERT INTO contacts(ring_id, username, alias, photo) values(?, ?, ?, ?)");

    if (not query_->prepare(addContactQuery)) {
        qDebug() << "DatabaseManager: addContact, " << query_->lastError().text();
        return;
    }

    query_->addBindValue(contact_id);
    query_->addBindValue(contact_id);
    // ^TODO: ring_id and username should be different
    query_->addBindValue(alias);
    query_->addBindValue(photo);

    if (not query_->exec()) {
        qDebug() << "DatabaseManager: addContact, " << query_->lastError().text();
        return;
    }

    auto account = AvailableAccountModel::instance().currentDefaultAccount();
    NameDirectory::instance().lookupName(account, QString(), contact_id);

    emit contactAdded(contact);
}

std::string
DatabaseManager::getUri(const std::string& uid) const
{
    auto getUriQuery = "SELECT username FROM contacts WHERE ring_id='" + uid + "'";

    if (not query_->exec(QString(getUriQuery.c_str()))) {
        qDebug() << "DatabaseManager: getUri, " << query_->lastError().text();
        return std::string();
    }

    query_->next();
    return query_->value(0).toString().toStdString();
}

std::string
DatabaseManager::getAlias(const std::string& uid) const
{
    auto getAliasQuery = "SELECT alias FROM contacts WHERE ring_id='" + uid + "'";

    if (not query_->exec(QString(getAliasQuery.c_str()))) {
        qDebug() << "DatabaseManager: getAlias, " << query_->lastError().text();
        return std::string();
    }

    query_->next();
    return query_->value(0).toString().toStdString();
}

std::string
DatabaseManager::getAvatar(const std::string& uid) const
{
    auto getAvatarQuery = "SELECT photo FROM contacts WHERE ring_id='" + uid + "'";

    if (not query_->exec(QString(getAvatarQuery.c_str()))) {
        qDebug() << "DatabaseManager: getAvatar, " << query_->lastError().text();
        return std::string();
    }

    query_->next();
    return query_->value(0).toString().toStdString();
}

void
DatabaseManager::slotRegisteredNameFound(const Account* account,
                                         NameDirectory::LookupStatus status,
                                         const QString& address,
                                         const QString& name)
{
    Q_UNUSED(account)
    Q_UNUSED(status)
    // For now, registeredNameFound is fired even if it not find any username.

    auto updateUserQuery = QString("UPDATE contacts SET username = '" + name + "' WHERE ring_id='" + address + "'");

    if (not query_->exec(updateUserQuery))
        qDebug() << "DatabaseManager: slotRegisteredNameFound, " << query_->lastError().text();

}
