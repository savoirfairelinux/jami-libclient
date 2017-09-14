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
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtCore/QStandardPaths>

// Lrc
#include "private/vcardutils.h"
#include "availableaccountmodel.h"

namespace lrc
{

using namespace api;

Database::Database()
: QObject()
{
    //migrateOldFiles();
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
    if (not tables.contains("contacts", Qt::CaseInsensitive)) {
        if (not query_->exec(DATABASE_CREATE_CONTACTS_TABLES))
            qDebug() << "Database: " << query_->lastError().text();

        auto photo = "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";
        const auto addContactQuery = QString(DATABASE_ADD_CONTACT(
          "", "", "",
          photo).c_str()
        );

        if (not query_->exec(addContactQuery)) {
            qDebug() << "Database: addContact, " << query_->lastError().text();
        }
    }

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

void
Database::migrateOldFiles()
{
    migrateTextHistory();
    migratePeerProfiles();
    migrateLocalProfiles();
}

void
Database::migrateTextHistory()
{
    // load all text recordings so we can recover CMs that are not in the call history
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/text/");
    if (dir.exists()) {
        // get .json files, sorted by time, latest first
        QStringList filters;
        filters << "*.json";
        auto list = dir.entryInfoList(filters, QDir::Files | QDir::NoSymLinks | QDir::Readable, QDir::Time);

        for (int i = 0; i < list.size(); ++i) {
            QFileInfo fileInfo = list.at(i);

            QString content;
            QFile file(fileInfo.absoluteFilePath());
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                content = QString::fromUtf8(file.readAll());
            } else {
                qWarning() << "Could not open text recording json file";
            }

            if (!content.isEmpty()) {
                QJsonParseError err;
                auto loadDoc = QJsonDocument::fromJson(content.toUtf8(), &err).object();

                for (const auto& key: loadDoc.keys()) {
                    if (key == "peers") {
                        auto peersObject = loadDoc[key].toArray()[0].toObject();
                        // TODO
                        qDebug() << "TODO: add conversation between "
                        << peersObject["accountId"].toString() << " and "
                        << peersObject["uri"].toString();
                    } else if (key == "groups") {
                        auto groupsArray = loadDoc[key].toArray();
                        for (const auto& groupObject: groupsArray) {
                            auto messagesArray = groupObject.toObject()["messages"].toArray();
                            for (const auto& messageRef: messagesArray) {
                                auto messageObject = messageRef.toObject();
                                // TODO
                                qDebug() << "TODO, add interaction, direction: " << messageObject["direction"].toInt()
                                << ", read:" << messageObject["isRead"].toInt()
                                << ", timestamp:" <<  messageObject["timestamp"].toInt()
                                << ", content: " << messageObject["payloads"].toArray()[0].toObject()["payload"].toString();
                            }
                        }
                    }
                }

            } else {
                qWarning() << "Text recording file is empty";
            }
            // TODO file.remove();
        }
    }
}

void
Database::migratePeerProfiles()
{
    const QDir profilesDir = (QStandardPaths::writableLocation(QStandardPaths::DataLocation)) + "/peer_profiles/";

    const QStringList entries = profilesDir.entryList({QStringLiteral("*.vcf")}, QDir::Files);

    foreach (const QString& item , entries) {
        auto filePath = profilesDir.path() + '/' + item;
        QString content;
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            content = QString::fromUtf8(file.readAll());
        } else {
            qWarning() << "Could not vcf file";
        }

        const auto vCard = VCardUtils::toHashMap(content.toUtf8());
        const auto uri = vCard["UID"];
        const auto alias = vCard["FN"];
        const auto photo = vCard["PHOTO;ENCODING=BASE64;TYPE=PNG"];

        qDebug() << "TODO add profiles " << uri << ", " << alias << ", " << photo;
        // TODO file.remove();
    }
}

void
Database::migrateLocalProfiles()
{
    const QDir profilesDir = (QStandardPaths::writableLocation(QStandardPaths::DataLocation)) + "/profiles/";

    const QStringList entries = profilesDir.entryList({QStringLiteral("*.vcf")}, QDir::Files);

    foreach (const QString& item , entries) {
        auto filePath = profilesDir.path() + '/' + item;
        QString content;
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            content = QString::fromUtf8(file.readAll());
        } else {
            qWarning() << "Could not vcf file";
        }

        auto personProfile = new Person(nullptr);
        QList<Account*> accs;
        VCardUtils::mapToPerson(personProfile, content.toUtf8(), &accs);
        const auto vCard = VCardUtils::toHashMap(content.toUtf8());
        const auto alias = vCard["FN"];
        const auto photo = vCard["PHOTO;ENCODING=BASE64;TYPE=PNG"];

        // TODO
        for (const auto& account: accs) {
            qDebug() << "TODO ADD PROFILE: " << account->id()
            << ", " << alias
            << ", " << photo;
        }

        // TODO file.remove();
    }
}


} // namespace lrc
