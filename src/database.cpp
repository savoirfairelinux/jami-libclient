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
#include "databasehelper.h"

// Qt
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

// Lrc
#include "private/vcardutils.h"
#include "availableaccountmodel.h"

namespace lrc
{

using namespace api;

Database::Database()
: QObject()
{
    if (not QSqlDatabase::drivers().contains("QSQLITE")) {
        qDebug() << "Database, errror QSQLITE not supported";
        return;
    }

    // TODO store the database in a standard path

    // initalize the database
    db_ = QSqlDatabase::addDatabase("QSQLITE");
    db_.setDatabaseName(ringDB);

    if (not db_.open()) {
        qDebug() << "Database, can't open the database";
        return;
    }

    // connect query object to database object
    // You must load the SQL driver and open the connection before a QSqlQuery is created.
    query_ = std::unique_ptr<QSqlQuery>(new QSqlQuery(db_));

    // check if all tables are presents:
    QStringList tables = db_.tables();

    // Set the version of the database
    if (tables.empty()) {
        auto storeVersionQuery = std::string(DATABASE_STORE_VERSION);
        storeVersionQuery += std::string(DATABASE_VERSION);
        if (not query_->exec(storeVersionQuery.c_str()))
            qDebug() << "Database: " << query_->lastError().text();
    }

    // add accounts table
    if (not tables.contains("accounts", Qt::CaseInsensitive))
        if (not query_->exec(DATABASE_CREATE_ACCOUNTS_TABLES))
            qDebug() << "Database: " << query_->lastError().text();

    // add contacts table
    if (not tables.contains("contacts", Qt::CaseInsensitive))
        if (not query_->exec(DATABASE_CREATE_CONTACTS_TABLES))
            qDebug() << "Database: " << query_->lastError().text();

    // add conversations table
    if (not tables.contains("conversations", Qt::CaseInsensitive))
        if (not query_->exec(DATABASE_CREATE_CONVERSATIONS_TABLES))
            qDebug() << "Database: " << query_->lastError().text();
}

Database::~Database()
{

}

void
Database::addMessage(const std::string& accountId, const message::Info& message) const
{
    std::string type;
    switch (message.type) {
    case message::Type::TEXT:
        type = "TEXT";
        break;
    case message::Type::CALL:
        type = "CALL";
        break;
    case message::Type::CONTACT:
        type = "CONTACT";
        break;
    case message::Type::INVALID:
        type = "INVALID";
        break;
    }
    std::string status;
    bool isOutgoing = true;
    switch (message.status) {
    case message::Status::SENDING:
        status = "SENDING";
        break;
    case message::Status::FAILED:
        status = "FAILED";
        break;
    case message::Status::SUCCEED:
        status = "SUCCEED";
        break;
    case message::Status::READ:
        status = "READ";
        isOutgoing = false;
        break;
    case message::Status::INVALID:
        status = "INVALID";
        break;
    }
    const auto addMessageQuery = DATABASE_ADD_MESSAGE(
        message.contact,
        accountId,
        message.body,
        std::to_string(message.timestamp),
        isOutgoing,
        type,
        status
    );

    if (not query_->exec(QString(addMessageQuery.c_str()))) {
        qDebug() << "Database: addMessage, " << query_->lastError().text();
        return;
    }

    if (not query_->exec(QString(DATABASE_GET_LAST_INSERTED_ID))) {
        qDebug() << "Database: getLastInsertedID, " << query_->lastError().text();
        return;
    }

    if (query_->next()) {
        emit messageAdded(query_->value(0).toInt(), accountId, message);
    }
}

void
Database::clearHistory(const std::string& account, const std::string& contactUri, bool removeContact) const
{
    auto clearHistoryQuery = DATABASE_CLEAR_HISTORY(account, contactUri);
    if (!removeContact) {
        clearHistoryQuery += " AND type!='CONTACT'";
    } else {
        // TODO link account and use profiles tabs
        auto removeContactQuery = "DELETE FROM contacts WHERE type='SIP' AND ring_id='" + contactUri + "'";
        if (not query_->exec(removeContactQuery.c_str())) {
            qDebug() << "Database: clearHistory, " << query_->lastError().text();
        }
    }

    if (not query_->exec(clearHistoryQuery.c_str())) {
        qDebug() << "Database: clearHistory, " << query_->lastError().text();
    }
}

MessagesMap
Database::getHistory(const std::string& accountId, const std::string& contactUri) const
{
    const auto getMessagesQuery = DATABASE_GET_MESSAGES(accountId, contactUri);

    if (not query_->exec(getMessagesQuery.c_str())) {
        qDebug() << "Database: getMessages, " << query_->lastError().text();
        return MessagesMap();
    }

    MessagesMap messages;
    while(query_->next()) {
        const auto message_id = query_->value(0).toInt();
        message::Info msg;
        msg.contact = query_->value(1).toString().toStdString();
        msg.body = query_->value(2).toString().toStdString();
        msg.timestamp = std::stoll(query_->value(3).toString().toStdString());
        const auto typeStr = query_->value(5).toString().toStdString();
        msg.type = message::Type::INVALID;
        if (typeStr == "TEXT") {
            msg.type = message::Type::TEXT;
        } else if (typeStr == "CALL") {
            msg.type = message::Type::CALL;
        } else if (typeStr == "CONTACT") {
            msg.type = message::Type::CONTACT;
        }
        const auto statusStr = query_->value(6).toString().toStdString();
        msg.status = message::Status::INVALID;
        if (statusStr == "SENDING") {
            msg.status = message::Status::SENDING;
        } else if (statusStr == "FAILED") {
            msg.status = message::Status::FAILED;
        } else if (statusStr == "SUCCEED") {
            msg.status = message::Status::SUCCEED;
        } else if (statusStr == "READ") {
            msg.status = message::Status::READ;
        }
        messages.insert(std::pair<int, message::Info>(message_id, msg));
    }

    return messages;
}

std::size_t
Database::numberOfUnreads(const std::string& account, const std::string& contactUri) const
{
    const auto numberOfUnreadsQuery = DATABASE_COUNT_MESSAGES_UNREAD(account, contactUri);

    if (not query_->exec(numberOfUnreadsQuery.c_str())) {
        qDebug() << "Database: NumberOfUnreads, " << query_->lastError().text();
        return -1;
    }

    if (query_->next()) {
        return query_->value(0).toUInt();
    }
    return 0;
}

void
Database::setMessageRead(int uid) const
{
    const auto setMessageReadQuery = DATABASE_SET_MESSAGE_READ(std::to_string(uid));

    if (not query_->exec(QString(setMessageReadQuery.c_str())))
        qDebug() << "Database: setMessageRead, " << query_->lastError().text();
}

void
Database::addContact(const std::string& contactUri, const QByteArray& payload) const
{
    // NOTE: this function will be improved
    const auto contact_id = QString(contactUri.c_str());
    const auto vCard = VCardUtils::toHashMap(payload);

    const auto alias = vCard["FN"];
    const auto photo = vCard["PHOTO;ENCODING=BASE64;TYPE=PNG"]; // TODO: to improve

    const auto addContactQuery = QString(DATABASE_ADD_CONTACT(
      contactUri, contactUri, alias.toStdString(),
      photo.toStdString()).c_str()
    );

    if (not query_->exec(addContactQuery)) {
        qDebug() << "Database: addContact, " << query_->lastError().text();
        return;
    }

    // Update name
    const auto account = AvailableAccountModel::instance().currentDefaultAccount();
    NameDirectory::instance().lookupName(account, QString(), contact_id);

    emit contactAdded(contactUri);
}

void
Database::addSIPContact(const std::string& contactUri) const
{
    const auto addContactQuery = "INSERT INTO contacts(ring_id, alias, type) values('" + contactUri + "', '" + contactUri + "', 'SIP')";

    if (not query_->exec(QString(addContactQuery.c_str()))) {
        qDebug() << "Database: addSIPContact, " << query_->lastError().text();
        return;
    }
    emit contactAdded(contactUri);
}

std::vector<std::string>
Database::getSIPContacts() const
{
    // TODO by account
    const auto getSIPContactsQuery =  "SELECT ring_id FROM contacts WHERE type='SIP'";

    if (not query_->exec(getSIPContactsQuery)) {
        qDebug() << "Database: getSIPContacts, " << query_->lastError().text();
        return {};
    }

    auto contacts = std::vector<std::string>();
    while(query_->next()) {
        contacts.emplace_back(query_->value(0).toString().toStdString());
    }

    return contacts;
}


std::string
Database::getContactAttribute(const std::string& contactUri, const std::string& attribute) const
{
    const auto attributeQuery = DATABASE_GET_CONTACT_ATTRIBUTE(contactUri, attribute);

    if (not query_->exec(QString(attributeQuery.c_str()))) {
        qDebug() << "Database: getContactAttribute, " << query_->lastError().text();
        return std::string();
    }

    if (query_->next()) {
        return query_->value(0).toString().toStdString();
    }
    return "";
}

void
Database::slotRegisteredNameFound(const Account* account,
                                 NameDirectory::LookupStatus status,
                                 const QString& address,
                                 const QString& name) const
{
    Q_UNUSED(account)
    Q_UNUSED(status)
    // For now, registeredNameFound is fired even if it not find any username.

    auto updateUserQuery = DATABASE_UPDATE_CONTACT(address.toStdString(),
                                                   name.toStdString());

    if (not query_->exec(QString(updateUserQuery.c_str())))
        qDebug() << "Database: slotRegisteredNameFound, " << query_->lastError().text();

}

} // namespace lrc
