/****************************************************************************
 *   Copyright (C) 2017-2019 Savoir-faire Linux Inc.                        *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
 *   Author: Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>       *
 *   Author: Kateryna Kostiuk <kateryna.kostiuk@savoirfairelinux.com>       *
 *   Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>         *
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

// daemon
#include <account_const.h>

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
#include <QtCore/QVariant>
#include <QDir>

// Std
#include <sstream>

// Data
#include "api/interaction.h"

// Lrc for migrations
#include "dbus/configurationmanager.h"
#include "vcard.h"
#include <account_const.h>

namespace lrc
{

using namespace api;

Database::Database(const QString& name, const QString& basePath)
    : QObject()
    , connectionName_(name)
    , basePath_(basePath)
{
    if (not QSqlDatabase::drivers().contains("QSQLITE")) {
        throw std::runtime_error("QSQLITE not supported");
    }

    // ring -> jami path migration
    if (!name.compare("ring")) {
        QDir dataDir(basePath_);
        // create data directory if not created yet
        dataDir.mkpath(basePath_);
        QDir oldDataDir(basePath_);
        oldDataDir.cdUp();
        oldDataDir = oldDataDir
                    .absolutePath()
#if defined(_WIN32) || defined(__APPLE__)
                    + "/ring";
#else
                    + "/gnome-ring";
#endif
        QStringList filesList = oldDataDir.entryList();
        QString filename;
        QDir dir;
        bool success = true;
        foreach (filename, filesList) {
          qDebug() << "Migrate " << oldDataDir.absolutePath() << "/" << filename
                   << " to " << dataDir.absolutePath() + "/" + filename;
          if (filename != "." && filename != "..") {
            success &= dir.rename(oldDataDir.absolutePath() + "/" + filename,
                                  dataDir.absolutePath() + "/" + filename);
          }
        }
        if (success) {
            // Remove old directory if the migration is successful.
            oldDataDir.removeRecursively();
        }
    }

    // initalize the database.
    db_ = QSqlDatabase::addDatabase("QSQLITE", connectionName_);

    auto databaseFile = QFileInfo(basePath_ + connectionName_ + ".db");
    QString databaseFileName = databaseFile.fileName();
    auto absoluteDir = databaseFile.absoluteDir();

#ifdef ENABLE_TEST
    databaseFullPath_ = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation)).filePath(databaseFileName);
#else
    // make sure the directory exists
    if (!absoluteDir.exists())
        absoluteDir.mkpath(".");
    databaseFullPath_ = absoluteDir.filePath(databaseFileName);
#endif
    db_.setDatabaseName(databaseFullPath_);
}

Database::~Database()
{
}

void
Database::remove()
{
    // close db and remove file
    if (db_.isOpen()) {
        db_.close();
    }
    QFile(databaseFullPath_).remove();
}

void
Database::load()
{
    // open the database.
    if (not db_.open()) {
        std::stringstream ss;
        ss << "cannot open database: " << connectionName_.toStdString();
        throw std::runtime_error(ss.str());
    }

    // if db is empty we create them.
    if (db_.tables().empty()) {
        try {
            QSqlDatabase::database(connectionName_).transaction();
            createTables();
            QSqlDatabase::database(connectionName_).commit();
        } catch (QueryError& e) {
            QSqlDatabase::database(connectionName_).rollback();
            throw std::runtime_error("Could not correctly create the database");
        }
    } else {
        migrateIfNeeded();
    }
}

void
Database::createTables()
{
    QSqlQuery query(db_);

    auto tableConversations  = "CREATE TABLE conversations ( \
                                    id INTEGER, \
                                    participant TEXT, \
                                    extra_data TEXT \
                                )";

    auto indexConversations = "CREATE INDEX `idx_conversations_uri` ON `conversations` (`participant`)";

    auto tableInteractions   = "CREATE TABLE interactions ( \
                                    id INTEGER PRIMARY KEY, \
                                    author TEXT, \
                                    conversation INTEGER, \
                                    timestamp INTEGER, \
                                    body TEXT, \
                                    type TEXT, \
                                    status TEXT, \
                                    is_read INTEGER, \
                                    daemon_id BIGINT, \
                                    extra_data TEXT, \
                                    FOREIGN KEY(conversation) REFERENCES conversations(id) \
                                )";

    auto indexInteractions = "CREATE INDEX `idx_interactions_uri` ON `interactions` (`author`)";

    // add conversations table
    if (!db_.tables().contains("conversations", Qt::CaseInsensitive)) {
        if (!query.exec(tableConversations) || ! query.exec(indexConversations)) {
            throw QueryError(query);
        }
    }

    // add interactions table
    if (!db_.tables().contains("interactions", Qt::CaseInsensitive)) {
        if (!query.exec(tableInteractions) || !query.exec(indexInteractions)) {
            throw QueryError(query);
        }
    }

    storeVersion(VERSION);
}

void
Database::migrateIfNeeded()
{
    try {
        std::string currentVersion = getVersion();
        if (currentVersion == VERSION) {
            return;
        }
        QSqlDatabase::database().transaction();
        migrateFromVersion(currentVersion);
        storeVersion(VERSION);
        QSqlDatabase::database().commit();
    } catch (QueryError& e) {
        QSqlDatabase::database().rollback();
        throw std::runtime_error("Could not correctly migrate the database");
    }
}

void
Database::migrateFromVersion(const std::string& currentVersion)
{
    (void)currentVersion;
}

void
Database::storeVersion(const std::string& version)
{
    QSqlQuery query(db_);;

    auto storeVersionQuery = std::string("PRAGMA user_version = ") + version;

    if (not query.exec(storeVersionQuery.c_str()))
        throw QueryError(query);
}

std::string
Database::getVersion()
{
    QSqlQuery query(db_);
    auto getVersionQuery = std::string("pragma user_version");
    if (not query.exec(getVersionQuery.c_str()))
        throw QueryError(query);
    query.first();
    return  query.value(0).toString().toStdString();
}

int
Database::insertInto(const std::string& table,                             // "tests"
                     const std::map<std::string, std::string>& bindCol,    // {{":id", "id"}, {":forename", "colforname"}, {":name", "colname"}}
                     const std::map<std::string, std::string>& bindsSet)   // {{":id", "7"}, {":forename", "alice"}, {":name", "cooper"}}
{
    QSqlQuery query(db_);
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
    QSqlQuery query(db_);

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
    QSqlQuery query(db_);
    std::string columnsSelect;

    auto prepareStr = std::string("SELECT " + select + " FROM " + table +
                                  (where.empty() ? "" : (" WHERE " + where)));
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

    return result;
}

int
Database::count(const std::string& count, // "id", "body", ...
                const std::string& table, // "tests"
                const std::string& where, // "contact=:name AND id=:id"
                const std::map<std::string, std::string>& bindsWhere) // {{":name", "toto"}, {":id", "65"}}
{
    QSqlQuery query(db_);
    std::string columnsSelect;
    auto prepareStr = std::string("SELECT count(" + count + ") FROM " + table + " WHERE " + where);
    query.prepare(prepareStr.c_str());

    for (const auto& entry : bindsWhere)
        query.bindValue(entry.first.c_str(), entry.second.c_str());

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
    QSqlQuery query(db_);

    auto prepareStr = std::string("DELETE FROM " + table + " WHERE " + where);
    query.prepare(prepareStr.c_str());

    for (const auto& entry : bindsWhere)
        query.bindValue(entry.first.c_str(), entry.second.c_str());

    if(not query.exec())
        throw QueryDeleteError(query, table, where, bindsWhere);
}

void
Database::truncateTable(const std::string& table)
{
    QSqlQuery query(db_);

    auto prepareStr = std::string("TRUNCATE TABLE " + table);
    query.prepare(prepareStr.c_str());

    if (not query.exec())
        throw QueryTruncateError(query, table);
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

Database::QueryTruncateError::QueryTruncateError(const QSqlQuery& query,
    const std::string& table)
    : QueryError(query)
    , table(table)
{}

std::string
Database::QueryTruncateError::details()
{
    std::ostringstream oss;
    oss << "paramaters sent :";
    oss << "table = " << table.c_str();
    return oss.str();
}

/*****************************************************************************
 *                                                                           *
 *                               LegacyDatabase                              *
 *                                                                           *
 ****************************************************************************/
LegacyDatabase::~LegacyDatabase()
{
    remove();
    // remove old LRC files
    QDir(basePath_ + "text/").removeRecursively();
    QDir(basePath_ + "profiles/").removeRecursively();
    QDir(basePath_ + "peer_profiles/").removeRecursively();
}

void
LegacyDatabase::load()
{
    // check if some migratable data exists
    if (not QFile(basePath_ + "ring.db").exists() &&
        not QDir(basePath_ + "text/").exists() &&
        not QDir(basePath_ + "profiles/").exists() &&
        not QDir(basePath_ + "peer_profiles/").exists()) {
        throw std::runtime_error("no data to migrate");
    }

    // open the database.
    if (not db_.open()) {
        std::stringstream ss;
        ss << "cannot open database: " << connectionName_.toStdString();
        throw std::runtime_error(ss.str());
    }

    // if db is empty we create them.
    if (db_.tables().empty()) {
        try {
            QSqlDatabase::database(connectionName_).transaction();
            createTables();
            QSqlDatabase::database(connectionName_).commit();
        } catch (QueryError& e) {
            QSqlDatabase::database(connectionName_).rollback();
            throw std::runtime_error("Could not correctly create the database");
        }
        migrateOldFiles();
    } else {
        migrateIfNeeded();
    }
}

void
LegacyDatabase::createTables()
{
    QSqlQuery query(db_);

    auto tableProfiles = "CREATE TABLE profiles (id INTEGER PRIMARY KEY,  \
                                                 uri TEXT NOT NULL,       \
                                                 alias TEXT,              \
                                                 photo TEXT,              \
                                                 type TEXT,               \
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

    auto tableProfileAccounts = "CREATE TABLE profiles_accounts (profile_id INTEGER NOT NULL,                    \
                                                                 account_id TEXT NOT NULL,                        \
                                                                 is_account TEXT,                                 \
                                                                 FOREIGN KEY(profile_id) REFERENCES profiles(id))";
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

    // add profiles accounts table
    if (not db_.tables().contains("profiles_accounts", Qt::CaseInsensitive)
        and not query.exec(tableProfileAccounts)) {
        throw QueryError(query);
    }

    storeVersion(VERSION);
}

void
LegacyDatabase::migrateOldFiles()
{
    migrateLocalProfiles();
    migratePeerProfiles();
    migrateTextHistory();
    linkRingProfilesWithAccounts(true);
}

void
LegacyDatabase::migrateLocalProfiles()
{
    const QDir profilesDir = basePath_ + "profiles/";
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

        const auto vCard = lrc::vCard::utils::toHashMap(content.toUtf8());
        const auto alias = vCard[lrc::vCard::Property::FORMATTED_NAME];
        const auto avatar = vCard["PHOTO;ENCODING=BASE64;TYPE=PNG"];

        const QStringList accountIds = ConfigurationManager::instance().getAccountList();
        for (auto accountId : accountIds) {
            MapStringString account = ConfigurationManager::instance().
            getAccountDetails(accountId.toStdString().c_str());
            auto accountURI = account[DRing::Account::ConfProperties::USERNAME].contains("ring:") ?
            account[DRing::Account::ConfProperties::USERNAME]
        .toStdString().substr(std::string("ring:").size()) :
        account[DRing::Account::ConfProperties::USERNAME].toStdString();

            for (const auto& accountId: accountIds) {
                MapStringString account = ConfigurationManager::instance()
                    .getAccountDetails(accountId.toStdString().c_str());
                auto type = account[DRing::Account::ConfProperties::TYPE] == "SIP"? "SIP" : "RING";

                auto uri = account[DRing::Account::ConfProperties::USERNAME].contains("ring:") ?
                        account[DRing::Account::ConfProperties::USERNAME]
                        .toStdString().substr(std::string("ring:").size()) :
                        account[DRing::Account::ConfProperties::USERNAME].toStdString();
                if (select("id", "profiles","uri=:uri", {{":uri", uri}}).payloads.empty()) {
                    insertInto("profiles",
                            {{":uri", "uri"}, {":alias", "alias"},
                            {":photo", "photo"}, {":type", "type"},
                            {":status", "status"}},
                            {{":uri", uri}, {":alias", alias.toStdString()},
                            {":photo", avatar.toStdString()}, {":type", type},
                            {":status", "TRUSTED"}});
                    auto profileIds = select("id", "profiles","uri=:uri",
                    {{":uri", uri}}).payloads;
                    if (!profileIds.empty() && select("profile_id", "profiles_accounts",
                    "account_id=:account_id AND is_account=:is_account",
                    {{":account_id", accountId.toStdString()},
                    {":is_account", "true"}}).payloads.empty()) {
                        insertInto("profiles_accounts",
                                    {{":profile_id", "profile_id"},
                                    {":account_id", "account_id"},
                                    {":is_account", "is_account"}},
                                    {{":profile_id", profileIds[0]},
                                    {":account_id", accountId.toStdString()},
                                    {":is_account", "true"}});
                    }
                }
            }
        }
    }
}

void
LegacyDatabase::migratePeerProfiles()
{
    const QDir profilesDir = basePath_ + "peer_profiles/";

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

        const auto vCard = lrc::vCard::utils::toHashMap(content.toUtf8());
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
LegacyDatabase::migrateTextHistory()
{
    // load all text recordings so we can recover CMs that are not in the call history
    QDir dir(basePath_ + "text/");
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

                MapStringString details = ConfigurationManager::instance().getAccountDetails(peersObject["accountId"].toString());
                if (!details.contains(DRing::Account::ConfProperties::USERNAME)) continue;

                auto accountUri = details[DRing::Account::ConfProperties::USERNAME];
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
                    // link profile id to account id
                    auto profiles = select("profile_id", "profiles_accounts",
                                            "profile_id=:profile_id AND \
                                            account_id=:account_id AND  \
                                            is_account=:is_account",
                                            {{":profile_id", contactId},
                                            {":account_id", peersObject["accountId"].toString().toStdString()},
                                            {":is_account", "false"}})
                                            .payloads;

                    if (profiles.empty()) {
                         insertInto("profiles_accounts",
                                    {{":profile_id", "profile_id"},
                                    {":account_id", "account_id"},
                                    {":is_account", "is_account"}},
                                    {{":profile_id", contactId},
                                    {":account_id", peersObject["accountId"].toString().toStdString()},
                                    {":is_account", "false"}});
                    }
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

void
LegacyDatabase::migrateFromVersion(const std::string& currentVersion)
{
    if (currentVersion == "1") {
        migrateSchemaFromVersion1();
    }
}

void
LegacyDatabase::migrateSchemaFromVersion1()
{
    QSqlQuery query(db_);
    auto tableProfileAccounts = "CREATE TABLE profiles_accounts (profile_id INTEGER NOT NULL,                     \
                                                                 account_id TEXT NOT NULL,                        \
                                                                 is_account TEXT,                                 \
                                                                 FOREIGN KEY(profile_id) REFERENCES profiles(id))";
    // add profiles accounts table
    if (not db_.tables().contains("profiles_accounts", Qt::CaseInsensitive)
        and not query.exec(tableProfileAccounts)) {
        throw QueryError(query);
    }
    linkRingProfilesWithAccounts(false);
}

void
LegacyDatabase::linkRingProfilesWithAccounts(bool contactsOnly)
{
    const QStringList accountIds =
    ConfigurationManager::instance().getAccountList();
    for (auto accountId : accountIds) {
        MapStringString account = ConfigurationManager::instance().
        getAccountDetails(accountId.toStdString().c_str());
        auto accountURI = account[DRing::Account::ConfProperties::USERNAME].contains("ring:") ?
        account[DRing::Account::ConfProperties::USERNAME]
       .toStdString().substr(std::string("ring:").size()) :
       account[DRing::Account::ConfProperties::USERNAME].toStdString();
        auto profileIds = select("id", "profiles","uri=:uri", {{":uri", accountURI}}).payloads;
        if(profileIds.empty()) {
            continue;
        }
        if(!contactsOnly) {
            //if is_account is true we should have only one profile id for account id
             if (select("profile_id", "profiles_accounts",
                        "account_id=:account_id AND is_account=:is_account",
                        {{":account_id", accountId.toStdString()},
                        {":is_account", "true"}}).payloads.empty()) {
                            insertInto("profiles_accounts",
                            {{":profile_id", "profile_id"}, {":account_id", "account_id"},
                            {":is_account", "is_account"}},
                            {{":profile_id", profileIds[0]}, {":account_id", accountId.toStdString()},
                            {":is_account", "true"}});
             }
        }

        if (account[DRing::Account::ConfProperties::TYPE] == DRing::Account::ProtocolNames::RING) {

            // update RING contacts
            const VectorMapStringString& contacts_vector = ConfigurationManager::instance()
           .getContacts(accountId.toStdString().c_str());
            //update contacts profiles
            for (auto contact_info : contacts_vector) {
                auto contactURI = contact_info["id"];
                updateProfileAccountForContact(contactURI.toStdString(), accountId.toStdString());
            }
            //update pending contacts profiles
            const VectorMapStringString& pending_tr = ConfigurationManager::instance()
            .getTrustRequests(accountId.toStdString().c_str());
            for (auto tr_info : pending_tr) {
                auto contactURI = tr_info[DRing::Account::TrustRequest::FROM];
                updateProfileAccountForContact(contactURI.toStdString(), accountId.toStdString());
            }
        } else if (account[DRing::Account::ConfProperties::TYPE] == DRing::Account::ProtocolNames::SIP) {
            // update SIP contacts
            auto conversations = select("id", "conversations",
                                        "participant_id=:participant_id",
                                        {{":participant_id", profileIds[0]}}).payloads;
            for (const auto& c : conversations) {
                auto otherParticipants = select("participant_id","conversations",
                                                "id=:id AND participant_id!=:participant_id",
                                                {{":id", c}, {":participant_id", profileIds[0]}})
                                                .payloads;
                for (const auto& participant: otherParticipants) {
                    auto rows = select("profile_id", "profiles_accounts",
                                       "profile_id=:profile_id AND \
                                        account_id=:account_id AND  \
                                        is_account=:is_account",
                                        {{":profile_id", participant},
                                        {":account_id", accountId.toStdString()},
                                        {":is_account", "false"}}).payloads;
                    if (rows.empty()) {
                        insertInto("profiles_accounts",
                        {{":profile_id", "profile_id"}, {":account_id", "account_id"},
                        {":is_account", "is_account"}},
                        {{":profile_id", participant}, {":account_id", accountId.toStdString()},
                        {":is_account", "false"}});
                    }
                }
            }
        }
    }
}

void
LegacyDatabase::updateProfileAccountForContact(const std::string& contactURI,
    const std::string& accountId)
{
    auto profileIds = select("id", "profiles", "uri=:uri",
        { {":uri", contactURI} })
        .payloads;
    if (profileIds.empty()) {
        return;
    }
    auto rows = select("profile_id", "profiles_accounts",
        "account_id=:account_id AND is_account=:is_account", { {":account_id", accountId},
        {":is_account", "false"} }).payloads;
    if (std::find(rows.begin(), rows.end(), profileIds[0]) == rows.end()) {
        insertInto("profiles_accounts",
            { {":profile_id", "profile_id"}, {":account_id", "account_id"},
            {":is_account", "is_account"} },
            { {":profile_id", profileIds[0]}, {":account_id", accountId},
            {":is_account", "false"} });
    }
}

} // namespace lrc
