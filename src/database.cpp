/****************************************************************************
 *   Copyright (C) 2017-2018 Savoir-faire Linux                                  *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
 *   Author: Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>       *
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
#include <QObject>
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include <QtCore/QStandardPaths>
#include <QtCore/QVariant>
#include <QDir>

// Std
#include <sstream>

// Data
#include "api/interaction.h"

// Lrc for migrations
#include "dbus/configurationmanager.h"
#include "person.h"
#include "account.h"
#include "accountmodel.h"
#include "private/vcardutils.h"

namespace lrc
{

using namespace api;

static constexpr auto VERSION = "1";
static constexpr auto NAME = "ring.db";

Database::Database()
: QObject()
{
    if (not QSqlDatabase::drivers().contains("QSQLITE")) {
        throw std::runtime_error("QSQLITE not supported");
    }

    {
        // create data directory if not created yet
        QDir dataDir;
        dataDir.mkpath(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    }

    // initalize the database.
    db_ = QSqlDatabase::addDatabase("QSQLITE");
#ifdef ENABLE_TEST
    db_.setDatabaseName(QString("/tmp/") + QString(NAME));
#else
    db_.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + NAME);
#endif

    // open the database.
    if (not db_.open()) {
        throw std::runtime_error("cannot open database");
    }

    // if db is empty we create them.
    if (db_.tables().empty()) {
        try {
            QSqlDatabase::database().transaction();
            createTables();
            QSqlDatabase::database().commit();
        } catch (QueryError& e) {
            QSqlDatabase::database().rollback();
            throw std::runtime_error("Could not correctly create the database");
        }
        // NOTE: the migration can take some time.
        migrateOldFiles();
    }
}

Database::~Database()
{

}

void
Database::createTables()
{
    QSqlQuery query;

    auto tableProfiles = "CREATE TABLE profiles (id INTEGER PRIMARY KEY,\
                                                 uri TEXT NOT NULL,     \
                                                 alias TEXT,            \
                                                 photo TEXT,            \
                                                 type TEXT,             \
                                                 status TEXT)";

    auto tableConversations = "CREATE TABLE conversations (id INTEGER,\
                                                           participant_id INTEGER, \
                                                           FOREIGN KEY(participant_id) REFERENCES profiles(id))";

    auto tableInteractions = "CREATE TABLE interactions (id INTEGER PRIMARY KEY,\
                                                         account_id INTEGER, \
                                                         author_id INTEGER, \
                                                         conversation_id INTEGER, \
                                                         timestamp INTEGER, \
                                                         body TEXT,     \
                                                         type TEXT,  \
                                                         status TEXT, \
                                                         daemon_id TEXT, \
                                                         FOREIGN KEY(account_id) REFERENCES profiles(id), \
                                                         FOREIGN KEY(author_id) REFERENCES profiles(id), \
                                                         FOREIGN KEY(conversation_id) REFERENCES conversations(id))";
    // add profiles table
    if (not db_.tables().contains("profiles", Qt::CaseInsensitive)
        and not query.exec(tableProfiles)) {
            throw QueryError(query);
    }

    // add conversations table
    if (not db_.tables().contains("conversations", Qt::CaseInsensitive)
        and not query.exec(tableConversations)) {
            throw QueryError(query);
    }

    // add interactions table
    if (not db_.tables().contains("interactions", Qt::CaseInsensitive)
        and not query.exec(tableInteractions)) {
            throw QueryError(query);
    }

    storeVersion(VERSION);
}

void
Database::storeVersion(const std::string& version)
{
    QSqlQuery query;

    auto storeVersionQuery = std::string("PRAGMA user_version = ") + version;

    if (not query.exec(storeVersionQuery.c_str()))
        throw QueryError(query);
}

int
Database::insertInto(const std::string& table,                             // "tests"
                     const std::map<std::string, std::string>& bindCol,    // {{":id", "id"}, {":forename", "colforname"}, {":name", "colname"}}
                     const std::map<std::string, std::string>& bindsSet)   // {{":id", "7"}, {":forename", "alice"}, {":name", "cooper"}}
{
    QSqlQuery query;
    std::string columns;
    std::string binds;

    for (const auto& entry : bindCol) {
        columns += entry.second + ",";
        binds += entry.first + ",";
    }

    // remove the last ','
    columns.pop_back();
    binds.pop_back();

    auto prepareStr = std::string("INSERT INTO " + table + " (" + columns + ") VALUES (" + binds + ")");
    query.prepare(prepareStr.c_str());

    for (const auto& entry : bindsSet)
        query.bindValue(entry.first.c_str(), entry.second.c_str());

    if (not query.exec())
        throw QueryInsertError(query, table, bindCol, bindsSet);

    if (not query.exec("SELECT last_insert_rowid()"))
        throw QueryInsertError(query, table, bindCol, bindsSet);

    if (!query.next())
        return -1;

    return query.value(0).toInt();
}

void
Database::update(const std::string& table,                              // "tests"
                 const std::string& set,                                // "location=:place, phone:=nmbr"
                 const std::map<std::string, std::string>& bindsSet,    // {{":place", "montreal"}, {":nmbr", "514"}}
                 const std::string& where,                              // "contact=:name AND id=:id
                 const std::map<std::string, std::string>& bindsWhere)  // {{":name", "toto"}, {":id", "65"}}
{
    QSqlQuery query;

    auto prepareStr = std::string("UPDATE " + table + " SET " + set + " WHERE " + where);
    query.prepare(prepareStr.c_str());

    for (const auto& entry : bindsSet)
        query.bindValue(entry.first.c_str(), entry.second.c_str());

    for (const auto& entry : bindsWhere)
        query.bindValue(entry.first.c_str(), entry.second.c_str());

    if (not query.exec())
        throw QueryUpdateError(query, table, set, bindsSet, where, bindsWhere);
}

Database::Result
Database::select(const std::string& select,                            // "id", "body", ...
                 const std::string& table,                             // "tests"
                 const std::string& where,                             // "contact=:name AND id=:id
                 const std::map<std::string, std::string>& bindsWhere) // {{":name", "toto"}, {":id", "65"}}
{
    QSqlQuery query;
    std::string columnsSelect;

    auto prepareStr = std::string("SELECT " + select + " FROM " + table + " WHERE " + where);
    query.prepare(prepareStr.c_str());

    for (const auto& entry : bindsWhere)
        query.bindValue(entry.first.c_str(), entry.second.c_str());

    if (not query.exec())
        throw QuerySelectError(query, select, table, where, bindsWhere);

    QSqlRecord rec = query.record();
    const auto col_num = rec.count();
    Database::Result result = {col_num, std::vector<std::string>()};

    // for each row
    while (query.next()) {
        for (int i = 0 ; i < col_num ; i++)
            result.payloads.emplace_back(query.value(i).toString().toStdString());
    }

    return std::move(result);
}

int
Database::count(const std::string& count, // "id", "body", ...
                const std::string& table, // "tests"
                const std::string& where) // "contact=:name AND id=:id"
{
    QSqlQuery query;
    std::string columnsSelect;
    auto prepareStr = std::string("SELECT count(" + count + ") FROM " + table + " WHERE " + where);
    query.prepare(prepareStr.c_str());

    if (not query.exec())
        throw QueryError(query);

    query.next();
    return query.value(0).toInt();
}

void
Database::deleteFrom(const std::string& table,                             // "tests"
                     const std::string& where,                             // "contact=:name AND id=:id
                     const std::map<std::string, std::string>& bindsWhere) // {{":name", "toto"}, {":id", "65"}}
{
    QSqlQuery query;

    auto prepareStr = std::string("DELETE FROM " + table + " WHERE " + where);
    query.prepare(prepareStr.c_str());

    for (const auto& entry : bindsWhere)
        query.bindValue(entry.first.c_str(), entry.second.c_str());

    if(not query.exec())
        throw QueryDeleteError(query, table, where, bindsWhere);
}

Database::QueryError::QueryError(const QSqlQuery& query)
    : std::runtime_error(query.lastError().text().toStdString())
    , query(query)
{}

Database::QueryInsertError::QueryInsertError(const QSqlQuery& query,
                                             const std::string& table,
                                             const std::map<std::string, std::string>& bindCol,
                                             const std::map<std::string, std::string>& bindsSet)
    : QueryError(query)
    , table(table), bindCol(bindCol), bindsSet(bindsSet)
{}

std::string
Database::QueryInsertError::details()
{
    std::ostringstream oss;
    oss << "paramaters sent :";
    oss << "table = " << table.c_str();
    for (auto& b : bindCol)
        oss << "   {" << b.first.c_str() << "}, {" << b.second.c_str() <<"}";
    for (auto& b : bindsSet)
        oss << "   {" << b.first.c_str() << "}, {" << b.second.c_str() <<"}";
    return oss.str();
}

Database::QueryUpdateError::QueryUpdateError(const QSqlQuery& query,
                                             const std::string& table,
                                             const std::string& set,
                                             const std::map<std::string, std::string>& bindsSet,
                                             const std::string& where,
                                             const std::map<std::string, std::string>& bindsWhere)
    : QueryError(query)
    , table(table), set(set), bindsSet(bindsSet), where(where), bindsWhere(bindsWhere)
{}

std::string
Database::QueryUpdateError::details()
{
    std::ostringstream oss;
    oss << "paramaters sent :";
    oss << "table = " << table.c_str();
    oss << "set = " << set.c_str();
    oss << "bindsSet :";
    for (auto& b : bindsSet)
        oss << "   {" << b.first.c_str() << "}, {" << b.second.c_str() <<"}";
    oss << "where = " << where.c_str();
    oss << "bindsWhere :";
    for (auto& b : bindsWhere)
        oss << "   {" << b.first.c_str() << "}, {" << b.second.c_str() <<"}";
    return oss.str();
}

Database::QuerySelectError::QuerySelectError(const QSqlQuery& query,
                                             const std::string& select,
                                             const std::string& table,
                                             const std::string& where,
                                             const std::map<std::string, std::string>& bindsWhere)
    : QueryError(query)
    , select(select), table(table), where(where), bindsWhere(bindsWhere)
{}

std::string
Database::QuerySelectError::details()
{
    std::ostringstream oss;
    oss << "paramaters sent :";
    oss << "select = " << select.c_str();
    oss << "table = " << table.c_str();
    oss << "where = " << where.c_str();
    oss << "bindsWhere :";
    for (auto& b : bindsWhere)
        oss << "   {" << b.first.c_str() << "}, {" << b.second.c_str() <<"}";
    return oss.str();
}

Database::QueryDeleteError::QueryDeleteError(const QSqlQuery& query,
                                             const std::string& table,
                                             const std::string& where,
                                             const std::map<std::string, std::string>& bindsWhere)
    : QueryError(query)
    , table(table), where(where), bindsWhere(bindsWhere)
{}

std::string
Database::QueryDeleteError::details()
{
    std::ostringstream oss;
    oss << "paramaters sent :";
    oss << "table = " << table.c_str();
    oss << "where = " << where.c_str();
    oss << "bindsWhere :";
    for (auto& b : bindsWhere)
        oss << "   {" << b.first.c_str() << "}, {" << b.second.c_str() <<"}";
    return oss.str();
}

void
Database::migrateOldFiles()
{
    migrateLocalProfiles();
    migratePeerProfiles();
    migrateTextHistory();
    // NOTE we don't remove old files for now.
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
            qWarning() << "Could not open .vcf file";
            continue;
        }

        auto personProfile = new Person(nullptr);
        QList<Account*> accs;
        VCardUtils::mapToPerson(personProfile, content.toUtf8(), &accs);
        const auto vCard = VCardUtils::toHashMap(content.toUtf8());
        // all accounts have the same profile picture for now.
        const auto alias = vCard["FN"];
        const auto avatar = vCard["PHOTO;ENCODING=BASE64;TYPE=PNG"];

        for (const auto& account: accs) {
            if (!account) continue;
            auto type = account->protocol() == Account::Protocol::RING ? "RING" : "SIP";
            auto uri = account->username();
            // Remove the ring: from the username because account uri is stored without "ring:" in the database
            if (uri.startsWith("ring:")) {
                uri = uri.mid(std::string("ring:").size());
            }
            if (select("id", "profiles","uri=:uri", {{":uri", uri.toStdString()}}).payloads.empty()) {
                insertInto("profiles",
                           {{":uri", "uri"}, {":alias", "alias"},
                           {":photo", "photo"}, {":type", "type"},
                           {":status", "status"}},
                           {{":uri", uri.toStdString()}, {":alias", alias.toStdString()},
                           {":photo", avatar.toStdString()}, {":type", type},
                           {":status", "TRUSTED"}});
            }
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
            qWarning() << "Could not open vcf file";
            continue;
        }

        const auto vCard = VCardUtils::toHashMap(content.toUtf8());
        auto uri = vCard["TEL;other"];
        const auto alias = vCard["FN"];
        const auto avatar = vCard["PHOTO;ENCODING=BASE64;TYPE=PNG"];
        const std::string type = uri.startsWith("ring:") ? "RING" : "SIP";
        if (uri.startsWith("ring:")) {
            uri = uri.mid(std::string("ring:").size());
        }

        if (select("id", "profiles","uri=:uri", {{":uri", uri.toStdString()}}).payloads.empty()) {
            insertInto("profiles",
                       {{":uri", "uri"}, {":alias", "alias"}, {":photo", "photo"}, {":type", "type"},
                       {":status", "status"}},
                       {{":uri", uri.toStdString()}, {":alias", alias.toStdString()},
                       {":photo", avatar.toStdString()}, {":type", type},
                       {":status", "TRUSTED"}});
        }

    }
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
                continue;
            }

            if (!content.isEmpty()) {
                QJsonParseError err;
                auto loadDoc = QJsonDocument::fromJson(content.toUtf8(), &err).object();

                if (loadDoc.find("peers") == loadDoc.end()) continue;
                if (loadDoc.find("groups") == loadDoc.end()) continue;
                // Load account
                auto peersObject = loadDoc["peers"].toArray()[0].toObject();
                auto account = AccountModel::instance().getById(peersObject["accountId"].toString().toUtf8());
                if (!account) continue;
                auto accountUri = account->username();
                auto isARingContact = accountUri.startsWith("ring:");
                if (isARingContact) {
                    accountUri = accountUri.mid(QString("ring:").length());
                }
                auto accountIds = select("id", "profiles","uri=:uri", {{":uri", accountUri.toStdString()}}).payloads;
                auto contactIds = select("id", "profiles","uri=:uri", {{":uri", peersObject["uri"].toString().toStdString()}}).payloads;
                if (contactIds.empty()) {
                    insertInto("profiles",
                               {{":uri", "uri"}, {":alias", "alias"}, {":photo", "photo"}, {":type", "type"},
                               {":status", "status"}},
                               {{":uri", peersObject["uri"].toString().toStdString()}, {":alias", ""},
                               {":photo", ""}, {":type", "RING"},
                               {":status", "TRUSTED"}});
                    // NOTE: this profile is in a case where it's not a contact for the daemon but a conversation with an account.
                    // So we choose to add the profile to daemon's contacts
                    if(isARingContact) {
                        ConfigurationManager::instance().addContact(
                            peersObject["accountId"].toString(),
                            peersObject["uri"].toString()
                        );
                    }
                    contactIds = select("id", "profiles","uri=:uri", {{":uri", peersObject["uri"].toString().toStdString()}}).payloads;
                }
                if (accountIds.empty()) {
                    qDebug() << "Can't find profile for URI: " << peersObject["accountId"].toString() << ". Ignore this file.";
                } else if (contactIds.empty()) {
                    qDebug() << "Can't find profile for URI: " << peersObject["uri"].toString() << ". Ignore this file.";
                } else {
                    auto contactId = contactIds[0];
                    auto accountId = accountIds[0];
                    auto newConversationsId = select("IFNULL(MAX(id), 0) + 1",
                                                        "conversations",
                                                        "1=1",
                                                        {}).payloads[0];
                    try
                    {
                        QSqlDatabase::database().transaction();
                        insertInto("conversations",
                                    {{":id", "id"}, {":participant_id", "participant_id"}},
                                    {{":id", newConversationsId}, {":participant_id", accountId}});
                        insertInto("conversations",
                                    {{":id", "id"}, {":participant_id", "participant_id"}},
                                    {{":id", newConversationsId}, {":participant_id", contactId}});
                        QSqlDatabase::database().commit();
                    } catch (QueryInsertError& e) {
                        qDebug() << e.details().c_str();
                        QSqlDatabase::database().rollback();
                    }

                    // Load interactions
                    auto groupsArray = loadDoc["groups"].toArray();
                    for (const auto& groupObject: groupsArray) {
                        auto messagesArray = groupObject.toObject()["messages"].toArray();
                        for (const auto& messageRef: messagesArray) {
                            auto messageObject = messageRef.toObject();
                            auto direction = messageObject["direction"].toInt();
                            auto body = messageObject["payloads"].toArray()[0].toObject()["payload"].toString();
                            insertInto("interactions",
                                        {{":account_id", "account_id"}, {":author_id", "author_id"},
                                        {":conversation_id", "conversation_id"}, {":timestamp", "timestamp"},
                                        {":body", "body"}, {":type", "type"},
                                        {":status", "status"}},
                                        {{":account_id", accountId}, {":author_id", direction ? accountId : contactId},
                                        {":conversation_id", newConversationsId},
                                        {":timestamp", std::to_string(messageObject["timestamp"].toInt())},
                                        {":body", body.toStdString()}, {":type", "TEXT"},
                                        {":status", direction ? "SUCCEED" : "READ"}});
                        }
                    }
                }
            } else {
                qWarning() << "Text recording file is empty";
            }
        }
    }
}


} // namespace lrc
