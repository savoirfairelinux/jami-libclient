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
#include "databasehelper.h"

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
    if (tables.empty()) {
        auto storeVersionQuery = std::string(DATABASE_STORE_VERSION);
        storeVersionQuery += std::string(DATABASE_VERSION);
        if (not query_->exec(storeVersionQuery.c_str()))
            qDebug() << "DatabaseManager: " << query_->lastError().text();
    }

    // add accounts table
    if (not tables.contains("accounts", Qt::CaseInsensitive))
        if (not query_->exec(DATABASE_CREATE_ACCOUNTS_TABLES))
            qDebug() << "DatabaseManager: " << query_->lastError().text();

    // add contacts table
    if (not tables.contains("contacts", Qt::CaseInsensitive))
        if (not query_->exec(DATABASE_CREATE_CONTACTS_TABLES))
            qDebug() << "DatabaseManager: " << query_->lastError().text();

    // add conversations table
    if (not tables.contains("conversations", Qt::CaseInsensitive))
        if (not query_->exec())
            qDebug() << "DatabaseManager: " << query_->lastError().text();
}

DatabaseManager::~DatabaseManager()
{

}

void
DatabaseManager::addMessage(const std::string& account, const Message::Info& message)
{
    std::string type;
    switch (message.type_) {
    case Message::Type::TEXT:
        type = "TEXT";
        break;
    case Message::Type::CALL:
        type = "CALL";
        break;
    case Message::Type::CONTACT:
        type = "CONTACT";
        break;
    case Message::Type::INVALID_TYPE:
        type = "INVALID_TYPE";
        break;
    }
    std::string status;
    switch (message.status_) {
    case Message::Status::SENDING:
        status = "SENDING";
        break;
    case Message::Status::FAILED:
        status = "FAILED";
        break;
    case Message::Status::SUCCEED:
        status = "SUCCEED";
        break;
    case Message::Status::INVALID_STATUS:
        status = "INVALID_STATUS";
        break;
    }
    auto addMessageQuery = DATABASE_ADD_MESSAGE(
        message.uid_,
        account,
        message.body_,
        std::to_string(message.timestamp_),
        message.isOutgoing_,
        type,
        status
    );

    if (not query_->exec(QString(addMessageQuery.c_str()))) {
        qDebug() << "DatabaseManager: addMessage, " << query_->lastError().text();
        return;
    }

    if (not query_->exec(QString(DATABASE_GET_LAST_INSERTED_ID))) {
        qDebug() << "DatabaseManager: getLastInsertedID, " << query_->lastError().text();
        return;
    }

    while(query_->next()) {
        emit messageAdded(query_->value(0).toInt(), account, message);
    }
}

void
DatabaseManager::removeHistory(const std::string& account, const std::string& uid, bool removeContact)
{
    auto removeHistoryQuery = DATABASE_REMOVE_HISTORY(account, uid);
    if (!removeContact) {
        removeHistoryQuery += " AND type!='CONTACT'";
    }

    if (not query_->exec(removeHistoryQuery.c_str())) {
        qDebug() << "DatabaseManager: removeHistory, " << query_->lastError().text();
    }
}

Messages
DatabaseManager::getMessages(const std::string& account, const std::string& uid) const
{
    auto getMessagesQuery = DATABASE_GET_MESSAGES(account, uid);

    if (not query_->exec(getMessagesQuery.c_str())) {
        qDebug() << "DatabaseManager: getMessages, " << query_->lastError().text();
        return Messages();
    }

    Messages messages;
    while(query_->next()) {
        auto message_id = query_->value(0).toInt();
        auto uid = query_->value(1).toString().toStdString();
        auto body = query_->value(2).toString().toStdString();
        std::time_t timestamp = std::stoll(query_->value(3).toString().toStdString());
        auto isOutgoing = query_->value(4).toBool();
        auto typeStr = query_->value(5).toString().toStdString();
        auto type = Message::Type::INVALID_TYPE;
        if (typeStr == "TEXT") {
            type = Message::Type::TEXT;
        } else if (typeStr == "CALL") {
            type = Message::Type::CALL;
        } else if (typeStr == "CONTACT") {
            type = Message::Type::CONTACT;
        }
        auto statusStr = query_->value(5).toString().toStdString();
        auto status = Message::Status::INVALID_STATUS;
        if (statusStr == "SENDING") {
            status = Message::Status::SENDING;
        } else if (statusStr == "FAILED") {
            status = Message::Status::FAILED;
        } else if (statusStr == "SUCCEED") {
            status = Message::Status::SUCCEED;
        }
        Message::Info msg(uid, body, isOutgoing, type, timestamp, status);
        messages.insert(std::pair<int, Message::Info>(message_id, msg));
    }

    return messages;
}

unsigned int
DatabaseManager::numberOfUnreads(const std::string& account, const std::string& uid) const
{
    auto numberOfUnreadsQuery = DATABASE_COUNT_MESSAGES_UNREAD(account, uid);

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
    auto setMessageReadQuery = QString(DATABASE_SET_MESSAGE_READ(std::to_string(uid)).c_str());

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

    auto addContactQuery = QString(DATABASE_ADD_CONTACT(
    contact_id.toStdString(), contact_id.toStdString(), alias.toStdString(),
    photo.toStdString()).c_str()
    );

    if (not query_->exec()) {
        qDebug() << "DatabaseManager: addContact, " << query_->lastError().text();
        return;
    }

    auto account = AvailableAccountModel::instance().currentDefaultAccount();
    NameDirectory::instance().lookupName(account, QString(), contact_id);

    emit contactAdded(contact);
}

std::string
DatabaseManager::getContactAttribute(const std::string& uid, const std::string& attribute) const
{
    auto attributeQuery = DATABASE_GET_CONTACT_ATTRIBUTE(uid, attribute);

    if (not query_->exec(QString(attributeQuery.c_str()))) {
        qDebug() << "DatabaseManager: getContactAttribute, " << query_->lastError().text();
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

    auto updateUserQuery = DATABASE_UPDATE_CONTACT(address.toStdString(),
                                                   name.toStdString());

    if (not query_->exec(QString(updateUserQuery.c_str())))
        qDebug() << "DatabaseManager: slotRegisteredNameFound, " << query_->lastError().text();

}
