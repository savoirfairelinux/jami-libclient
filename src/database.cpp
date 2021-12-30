/****************************************************************************
 *   Copyright (C) 2017-2021 Savoir-faire Linux Inc.                        *
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

#include "api/interaction.h"

#include <account_const.h>

// Lrc for migrations
#include "dbus/configurationmanager.h"
#include "vcard.h"
#include <account_const.h>

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
#include <QTextStream>

#include <sstream>
#include <stdexcept>

namespace lrc {

using namespace api;

Database::Database(const QString& name, const QString& basePath)
    : QObject()
    , connectionName_(name)
    , basePath_(basePath)
    , version_(DB_VERSION)
{
    if (not QSqlDatabase::drivers().contains("QSQLITE")) {
        throw std::runtime_error("QSQLITE not supported");
    }

    // initalize the database.
    db_ = QSqlDatabase::addDatabase("QSQLITE", connectionName_);

    auto databaseFile = QFileInfo(basePath_ + connectionName_ + ".db");
    QString databaseFileName = databaseFile.fileName();
    auto absoluteDir = databaseFile.absoluteDir();

    // make sure the directory exists
    if (!absoluteDir.exists())
        absoluteDir.mkpath(".");
    databaseFullPath_ = absoluteDir.filePath(databaseFileName);

    db_.setDatabaseName(databaseFullPath_);
}

Database::~Database()
{
    close();
}

void
Database::close()
{
    // close db
    if (db_.isOpen()) {
        db_.close();
    }
}

void
Database::remove()
{
    // close db and remove file
    close();
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

    auto tableConversations = "CREATE TABLE conversations ( \
                                    id INTEGER, \
                                    participant TEXT, \
                                    extra_data TEXT \
                                )";

    auto indexConversations
        = "CREATE INDEX `idx_conversations_uri` ON `conversations` (`participant`)";

    auto tableInteractions = "CREATE TABLE interactions ( \
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
        if (!query.exec(tableConversations) || !query.exec(indexConversations)) {
            throw QueryError(std::move(query));
        }
    }

    // add interactions table
    if (!db_.tables().contains("interactions", Qt::CaseInsensitive)) {
        if (!query.exec(tableInteractions) || !query.exec(indexInteractions)) {
            throw QueryError(std::move(query));
        }
    }

    storeVersion(version_);
}

void
Database::migrateIfNeeded()
{
    try {
        auto currentVersion = getVersion();
        if (currentVersion == version_) {
            return;
        }
        QSqlDatabase::database().transaction();
        migrateFromVersion(currentVersion);
        storeVersion(version_);
        QSqlDatabase::database().commit();
    } catch (QueryError& e) {
        QSqlDatabase::database().rollback();
        throw std::runtime_error("Could not correctly migrate the database");
    }
}

void
Database::migrateFromVersion(const QString& currentVersion)
{
    (void) currentVersion;
}

void
Database::storeVersion(const QString& version)
{
    QSqlQuery query(db_);

    auto storeVersionQuery = "PRAGMA user_version = " + version;

    if (not query.exec(storeVersionQuery))
        throw QueryError(std::move(query));

    qDebug() << "database " << databaseFullPath_ << " version set to:" << version;
}

QString
Database::getVersion()
{
    QSqlQuery query(db_);
    auto getVersionQuery = "pragma user_version";
    if (not query.exec(getVersionQuery))
        throw QueryError(std::move(query));
    query.first();
    return query.value(0).toString();
}

QString
Database::insertInto(
    const QString& table, // "tests"
    const MapStringString&
        bindCol, // {{":id", "id"}, {":forename", "colforname"}, {":name", "colname"}}
    const MapStringString& bindsSet) // {{":id", "7"}, {":forename", "alice"}, {":name", "cooper"}}
{
    QSqlQuery query(db_);
    QString columns;
    QString binds;

    for (const auto& entry : bindCol.toStdMap()) {
        columns += entry.second + ",";
        binds += entry.first + ",";
    }

    // remove the last ','
    columns.chop(1);
    binds.chop(1);

    auto prepareStr = "INSERT INTO " + table + " (" + columns + ") VALUES (" + binds + ")";
    query.prepare(prepareStr);

    for (const auto& entry : bindsSet.toStdMap())
        query.bindValue(entry.first, entry.second);

    if (not query.exec())
        throw QueryInsertError(std::move(query), table, bindCol, bindsSet);

    if (not query.exec("SELECT last_insert_rowid()"))
        throw QueryInsertError(std::move(query), table, bindCol, bindsSet);

    if (!query.next())
        return QString::number(-1);
    ;

    return query.value(0).toString();
}

void
Database::update(const QString& table,              // "tests"
                 const QString& set,                // "location=:place, phone:=nmbr"
                 const MapStringString& bindsSet,   // {{":place", "montreal"}, {":nmbr", "514"}}
                 const QString& where,              // "contact=:name AND id=:id
                 const MapStringString& bindsWhere) // {{":name", "toto"}, {":id", "65"}}
{
    QSqlQuery query(db_);

    auto prepareStr = QString("UPDATE " + table + " SET " + set + " WHERE " + where);
    query.prepare(prepareStr);

    for (const auto& entry : bindsSet.toStdMap())
        query.bindValue(entry.first, entry.second);

    for (const auto& entry : bindsWhere.toStdMap())
        query.bindValue(entry.first, entry.second);

    if (not query.exec())
        throw QueryUpdateError(std::move(query), table, set, bindsSet, where, bindsWhere);
}

Database::Result
Database::select(const QString& select,             // "id", "body", ...
                 const QString& table,              // "tests"
                 const QString& where,              // "contact=:name AND id=:id
                 const MapStringString& bindsWhere) // {{":name", "toto"}, {":id", "65"}}
{
    QSqlQuery query(db_);
    QString columnsSelect;

    auto prepareStr = QString("SELECT " + select + " FROM " + table
                              + (where.isEmpty() ? "" : (" WHERE " + where)));
    query.prepare(prepareStr);

    for (const auto& entry : bindsWhere.toStdMap())
        query.bindValue(entry.first, entry.second);

    if (not query.exec())
        throw QuerySelectError(std::move(query), select, table, where, bindsWhere);

    QSqlRecord rec = query.record();
    const auto col_num = rec.count();
    Database::Result result = {col_num, {}};

    // for each row
    while (query.next()) {
        for (int i = 0; i < col_num; i++)
            result.payloads.push_back(query.value(i).toString());
    }

    return result;
}

int
Database::count(const QString& count,              // "id", "body", ...
                const QString& table,              // "tests"
                const QString& where,              // "contact=:name AND id=:id"
                const MapStringString& bindsWhere) // {{":name", "toto"}, {":id", "65"}}
{
    QSqlQuery query(db_);
    QString columnsSelect;
    auto prepareStr = QString("SELECT count(" + count + ") FROM " + table + " WHERE " + where);
    query.prepare(prepareStr);

    for (const auto& entry : bindsWhere.toStdMap())
        query.bindValue(entry.first, entry.second);

    if (not query.exec())
        throw QueryError(std::move(query));

    query.next();
    return query.value(0).toInt();
}

void
Database::deleteFrom(const QString& table,              // "tests"
                     const QString& where,              // "contact=:name AND id=:id
                     const MapStringString& bindsWhere) // {{":name", "toto"}, {":id", "65"}}
{
    QSqlQuery query(db_);

    auto prepareStr = QString("DELETE FROM " + table + " WHERE " + where);
    query.prepare(prepareStr);

    for (const auto& entry : bindsWhere.toStdMap())
        query.bindValue(entry.first, entry.second);

    if (not query.exec())
        throw QueryDeleteError(std::move(query), table, where, bindsWhere);
}

Database::QueryError::QueryError(QSqlQuery&& query)
    : std::runtime_error(query.lastError().text().toStdString())
    , query(std::move(query))
{}

Database::QueryInsertError::QueryInsertError(QSqlQuery&& query,
                                             const QString& table,
                                             const MapStringString& bindCol,
                                             const MapStringString& bindsSet)
    : QueryError(std::move(query))
    , table(table)
    , bindCol(bindCol)
    , bindsSet(bindsSet)
{}

QString
Database::QueryInsertError::details()
{
    QTextStream qts;
    qts << "parameters sent :";
    qts << "table = " << table;
    for (auto& b : bindCol.toStdMap())
        qts << "   {" << b.first << "}, {" << b.second << "}";
    for (auto& b : bindsSet.toStdMap())
        qts << "   {" << b.first << "}, {" << b.second << "}";
    return qts.readAll();
}

Database::QueryUpdateError::QueryUpdateError(QSqlQuery&& query,
                                             const QString& table,
                                             const QString& set,
                                             const MapStringString& bindsSet,
                                             const QString& where,
                                             const MapStringString& bindsWhere)
    : QueryError(std::move(query))
    , table(table)
    , set(set)
    , bindsSet(bindsSet)
    , where(where)
    , bindsWhere(bindsWhere)
{}

QString
Database::QueryUpdateError::details()
{
    QTextStream qts;
    qts << "parameters sent :";
    qts << "table = " << table;
    qts << "set = " << set;
    qts << "bindsSet :";
    for (auto& b : bindsSet.toStdMap())
        qts << "   {" << b.first << "}, {" << b.second << "}";
    qts << "where = " << where;
    qts << "bindsWhere :";
    for (auto& b : bindsWhere.toStdMap())
        qts << "   {" << b.first << "}, {" << b.second << "}";
    return qts.readAll();
}

Database::QuerySelectError::QuerySelectError(QSqlQuery&& query,
                                             const QString& select,
                                             const QString& table,
                                             const QString& where,
                                             const MapStringString& bindsWhere)
    : QueryError(std::move(query))
    , select(select)
    , table(table)
    , where(where)
    , bindsWhere(bindsWhere)
{}

QString
Database::QuerySelectError::details()
{
    QTextStream qts;
    qts << "parameters sent :";
    qts << "select = " << select;
    qts << "table = " << table;
    qts << "where = " << where;
    qts << "bindsWhere :";
    for (auto& b : bindsWhere.toStdMap())
        qts << "   {" << b.first << "}, {" << b.second << "}";
    return qts.readAll();
}

Database::QueryDeleteError::QueryDeleteError(QSqlQuery&& query,
                                             const QString& table,
                                             const QString& where,
                                             const MapStringString& bindsWhere)
    : QueryError(std::move(query))
    , table(table)
    , where(where)
    , bindsWhere(bindsWhere)
{}

QString
Database::QueryDeleteError::details()
{
    QTextStream qts;
    qts << "parameters sent :";
    qts << "table = " << table;
    qts << "where = " << where;
    qts << "bindsWhere :";
    for (auto& b : bindsWhere.toStdMap())
        qts << "   {" << b.first << "}, {" << b.second << "}";
    return qts.readAll();
}

Database::QueryTruncateError::QueryTruncateError(QSqlQuery&& query, const QString& table)
    : QueryError(std::move(query))
    , table(table)
{}

QString
Database::QueryTruncateError::details()
{
    QTextStream qts;
    qts << "parameters sent :";
    qts << "table = " << table;
    return qts.readAll();
}

/*****************************************************************************
 *                                                                           *
 *                               LegacyDatabase                              *
 *                                                                           *
 ****************************************************************************/
LegacyDatabase::LegacyDatabase(const QString& basePath)
    : Database("ring", basePath)
{
    version_ = LEGACY_DB_VERSION;
}

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

    auto tableProfileAccounts
        = "CREATE TABLE profiles_accounts (profile_id INTEGER NOT NULL,                    \
                                                                 account_id TEXT NOT NULL,                        \
                                                                 is_account TEXT,                                 \
                                                                 FOREIGN KEY(profile_id) REFERENCES profiles(id))";
    // add profiles table
    if (not db_.tables().contains("profiles", Qt::CaseInsensitive)
        and not query.exec(tableProfiles)) {
        throw QueryError(std::move(query));
    }

    // add conversations table
    if (not db_.tables().contains("conversations", Qt::CaseInsensitive)
        and not query.exec(tableConversations)) {
        throw QueryError(std::move(query));
    }

    // add interactions table
    if (not db_.tables().contains("interactions", Qt::CaseInsensitive)
        and not query.exec(tableInteractions)) {
        throw QueryError(std::move(query));
    }

    // add profiles accounts table
    if (not db_.tables().contains("profiles_accounts", Qt::CaseInsensitive)
        and not query.exec(tableProfileAccounts)) {
        throw QueryError(std::move(query));
    }

    storeVersion(version_);
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
    foreach (const QString& item, entries) {
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
            // NOTE: If the daemon is down, but dbus answered, id can contains
            // "Remote peer disconnected", "The name is not activable", etc.
            // So avoid to migrate useless directories.
            for (auto& id : accountIds)
                if (id.indexOf(" ") != -1) {
                    qWarning() << "Invalid dbus answer. Daemon not running";
                    return;
                }
            MapStringString account = ConfigurationManager::instance().getAccountDetails(
                accountId.toStdString().c_str());
            auto accountURI = account[DRing::Account::ConfProperties::USERNAME].contains("ring:")
                                  ? account[DRing::Account::ConfProperties::USERNAME]
                                        .toStdString()
                                        .substr(std::string("ring:").size())
                                  : account[DRing::Account::ConfProperties::USERNAME].toStdString();

            for (const auto& accountId : accountIds) {
                MapStringString account = ConfigurationManager::instance().getAccountDetails(
                    accountId.toStdString().c_str());
                auto type = account[DRing::Account::ConfProperties::TYPE] == "SIP" ? "SIP" : "RING";

                auto uri = account[DRing::Account::ConfProperties::USERNAME].contains("ring:")
                               ? QString(account[DRing::Account::ConfProperties::USERNAME])
                                     .remove(0, QString("ring:").size())
                               : account[DRing::Account::ConfProperties::USERNAME];
                if (select("id", "profiles", "uri=:uri", {{":uri", uri}}).payloads.empty()) {
                    insertInto("profiles",
                               {{":uri", "uri"},
                                {":alias", "alias"},
                                {":photo", "photo"},
                                {":type", "type"},
                                {":status", "status"}},
                               {{":uri", uri},
                                {":alias", alias},
                                {":photo", avatar},
                                {":type", type},
                                {":status", "TRUSTED"}});
                    auto profileIds = select("id", "profiles", "uri=:uri", {{":uri", uri}}).payloads;
                    if (!profileIds.empty()
                        && select("profile_id",
                                  "profiles_accounts",
                                  "account_id=:account_id AND is_account=:is_account",
                                  {{":account_id", accountId}, {":is_account", "true"}})
                               .payloads.empty()) {
                        insertInto("profiles_accounts",
                                   {{":profile_id", "profile_id"},
                                    {":account_id", "account_id"},
                                    {":is_account", "is_account"}},
                                   {{":profile_id", profileIds[0]},
                                    {":account_id", accountId},
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

    foreach (const QString& item, entries) {
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
        const QString type = uri.startsWith("ring:") ? "RING" : "SIP";
        if (uri.startsWith("ring:")) {
            uri = uri.mid(QString("ring:").size());
        }

        if (select("id", "profiles", "uri=:uri", {{":uri", uri}}).payloads.empty()) {
            insertInto("profiles",
                       {{":uri", "uri"},
                        {":alias", "alias"},
                        {":photo", "photo"},
                        {":type", "type"},
                        {":status", "status"}},
                       {{":uri", uri},
                        {":alias", alias},
                        {":photo", avatar},
                        {":type", type},
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
        auto list = dir.entryInfoList(filters,
                                      QDir::Files | QDir::NoSymLinks | QDir::Readable,
                                      QDir::Time);

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

                if (loadDoc.find("peers") == loadDoc.end())
                    continue;
                if (loadDoc.find("groups") == loadDoc.end())
                    continue;
                // Load account
                auto peersObject = loadDoc["peers"].toArray()[0].toObject();

                MapStringString details = ConfigurationManager::instance().getAccountDetails(
                    peersObject["accountId"].toString());
                if (!details.contains(DRing::Account::ConfProperties::USERNAME))
                    continue;

                auto accountUri = details[DRing::Account::ConfProperties::USERNAME];
                auto isARingContact = accountUri.startsWith("ring:");
                if (isARingContact) {
                    accountUri = accountUri.mid(QString("ring:").length());
                }
                auto accountIds = select("id", "profiles", "uri=:uri", {{":uri", accountUri}})
                                      .payloads;
                auto contactIds = select("id",
                                         "profiles",
                                         "uri=:uri",
                                         {{":uri", peersObject["uri"].toString()}})
                                      .payloads;
                if (contactIds.empty()) {
                    insertInto("profiles",
                               {{":uri", "uri"},
                                {":alias", "alias"},
                                {":photo", "photo"},
                                {":type", "type"},
                                {":status", "status"}},
                               {{":uri", peersObject["uri"].toString()},
                                {":alias", ""},
                                {":photo", ""},
                                {":type", "RING"},
                                {":status", "TRUSTED"}});
                    // NOTE: this profile is in a case where it's not a contact for the daemon but a
                    // conversation with an account. So we choose to add the profile to daemon's contacts
                    if (isARingContact) {
                        ConfigurationManager::instance()
                            .addContact(peersObject["accountId"].toString(),
                                        peersObject["uri"].toString());
                    }
                    contactIds = select("id",
                                        "profiles",
                                        "uri=:uri",
                                        {{":uri", peersObject["uri"].toString()}})
                                     .payloads;
                }
                if (accountIds.empty()) {
                    qDebug() << "Can't find profile for URI: "
                             << peersObject["accountId"].toString() << ". Ignore this file.";
                } else if (contactIds.empty()) {
                    qDebug() << "Can't find profile for URI: " << peersObject["uri"].toString()
                             << ". Ignore this file.";
                } else {
                    auto contactId = contactIds[0];
                    // link profile id to account id
                    auto profiles = select("profile_id",
                                           "profiles_accounts",
                                           "profile_id=:profile_id AND \
                                            account_id=:account_id AND  \
                                            is_account=:is_account",
                                           {{":profile_id", contactId},
                                            {":account_id", peersObject["accountId"].toString()},
                                            {":is_account", "false"}})
                                        .payloads;

                    if (profiles.empty()) {
                        insertInto("profiles_accounts",
                                   {{":profile_id", "profile_id"},
                                    {":account_id", "account_id"},
                                    {":is_account", "is_account"}},
                                   {{":profile_id", contactId},
                                    {":account_id", peersObject["accountId"].toString()},
                                    {":is_account", "false"}});
                    }
                    auto accountId = accountIds[0];
                    auto newConversationsId
                        = select("IFNULL(MAX(id), 0) + 1", "conversations", "1=1", {}).payloads[0];
                    try {
                        QSqlDatabase::database().transaction();
                        insertInto("conversations",
                                   {{":id", "id"}, {":participant_id", "participant_id"}},
                                   {{":id", newConversationsId}, {":participant_id", accountId}});
                        insertInto("conversations",
                                   {{":id", "id"}, {":participant_id", "participant_id"}},
                                   {{":id", newConversationsId}, {":participant_id", contactId}});
                        QSqlDatabase::database().commit();
                    } catch (QueryInsertError& e) {
                        qDebug() << e.details();
                        QSqlDatabase::database().rollback();
                    }

                    // Load interactions
                    auto groupsArray = loadDoc["groups"].toArray();
                    for (const auto& groupObject : groupsArray) {
                        auto messagesArray = groupObject.toObject()["messages"].toArray();
                        for (const auto& messageRef : messagesArray) {
                            auto messageObject = messageRef.toObject();
                            auto direction = messageObject["direction"].toInt();
                            auto body = messageObject["payloads"]
                                            .toArray()[0]
                                            .toObject()["payload"]
                                            .toString();
                            insertInto("interactions",
                                       {{":account_id", "account_id"},
                                        {":author_id", "author_id"},
                                        {":conversation_id", "conversation_id"},
                                        {":timestamp", "timestamp"},
                                        {":body", "body"},
                                        {":type", "type"},
                                        {":status", "status"}},
                                       {{":account_id", accountId},
                                        {":author_id", direction ? accountId : contactId},
                                        {":conversation_id", newConversationsId},
                                        {":timestamp", messageObject["timestamp"].toString()},
                                        {":body", body},
                                        {":type", "TEXT"},
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
LegacyDatabase::migrateFromVersion(const QString& currentVersion)
{
    if (currentVersion == "1") {
        migrateSchemaFromVersion1();
    }
}

void
LegacyDatabase::migrateSchemaFromVersion1()
{
    QSqlQuery query(db_);
    auto tableProfileAccounts
        = "CREATE TABLE profiles_accounts (profile_id INTEGER NOT NULL,                     \
                                                                 account_id TEXT NOT NULL,                        \
                                                                 is_account TEXT,                                 \
                                                                 FOREIGN KEY(profile_id) REFERENCES profiles(id))";
    // add profiles accounts table
    if (not db_.tables().contains("profiles_accounts", Qt::CaseInsensitive)
        and not query.exec(tableProfileAccounts)) {
        throw QueryError(std::move(query));
    }
    linkRingProfilesWithAccounts(false);
}

void
LegacyDatabase::linkRingProfilesWithAccounts(bool contactsOnly)
{
    const QStringList accountIds = ConfigurationManager::instance().getAccountList();
    for (auto accountId : accountIds) {
        // NOTE: If the daemon is down, but dbus answered, id can contains
        // "Remote peer disconnected", "The name is not activable", etc.
        // So avoid to migrate useless directories.
        for (auto& id : accountIds)
            if (id.indexOf(" ") != -1) {
                qWarning() << "Invalid dbus answer. Daemon not running";
                return;
            }
        MapStringString account = ConfigurationManager::instance().getAccountDetails(
            accountId.toStdString().c_str());
        auto accountURI = account[DRing::Account::ConfProperties::USERNAME].contains("ring:")
                              ? QString(account[DRing::Account::ConfProperties::USERNAME])
                                    .remove(0, QString("ring:").size())
                              : account[DRing::Account::ConfProperties::USERNAME];
        auto profileIds = select("id", "profiles", "uri=:uri", {{":uri", accountURI}}).payloads;
        if (profileIds.empty()) {
            continue;
        }
        if (!contactsOnly) {
            // if is_account is true we should have only one profile id for account id
            if (select("profile_id",
                       "profiles_accounts",
                       "account_id=:account_id AND is_account=:is_account",
                       {{":account_id", accountId}, {":is_account", "true"}})
                    .payloads.empty()) {
                insertInto("profiles_accounts",
                           {{":profile_id", "profile_id"},
                            {":account_id", "account_id"},
                            {":is_account", "is_account"}},
                           {{":profile_id", profileIds[0]},
                            {":account_id", accountId},
                            {":is_account", "true"}});
            }
        }

        if (account[DRing::Account::ConfProperties::TYPE] == DRing::Account::ProtocolNames::RING) {
            // update RING contacts
            const VectorMapStringString& contacts_vector
                = ConfigurationManager::instance().getContacts(accountId.toStdString().c_str());
            // update contacts profiles
            for (auto contact_info : contacts_vector) {
                auto contactURI = contact_info["id"];
                updateProfileAccountForContact(contactURI, accountId);
            }
            // update pending contacts profiles
            const VectorMapStringString& pending_tr
                = ConfigurationManager::instance().getTrustRequests(accountId.toStdString().c_str());
            for (auto tr_info : pending_tr) {
                auto contactURI = tr_info[DRing::Account::TrustRequest::FROM];
                updateProfileAccountForContact(contactURI, accountId);
            }
        } else if (account[DRing::Account::ConfProperties::TYPE]
                   == DRing::Account::ProtocolNames::SIP) {
            // update SIP contacts
            auto conversations = select("id",
                                        "conversations",
                                        "participant_id=:participant_id",
                                        {{":participant_id", profileIds[0]}})
                                     .payloads;
            for (const auto& c : conversations) {
                auto otherParticipants = select("participant_id",
                                                "conversations",
                                                "id=:id AND participant_id!=:participant_id",
                                                {{":id", c}, {":participant_id", profileIds[0]}})
                                             .payloads;
                for (const auto& participant : otherParticipants) {
                    auto rows = select("profile_id",
                                       "profiles_accounts",
                                       "profile_id=:profile_id AND \
                                        account_id=:account_id AND  \
                                        is_account=:is_account",
                                       {{":profile_id", participant},
                                        {":account_id", accountId},
                                        {":is_account", "false"}})
                                    .payloads;
                    if (rows.empty()) {
                        insertInto("profiles_accounts",
                                   {{":profile_id", "profile_id"},
                                    {":account_id", "account_id"},
                                    {":is_account", "is_account"}},
                                   {{":profile_id", participant},
                                    {":account_id", accountId},
                                    {":is_account", "false"}});
                    }
                }
            }
        }
    }
}

void
LegacyDatabase::updateProfileAccountForContact(const QString& contactURI, const QString& accountId)
{
    auto profileIds = select("id", "profiles", "uri=:uri", {{":uri", contactURI}}).payloads;
    if (profileIds.empty()) {
        return;
    }
    auto rows = select("profile_id",
                       "profiles_accounts",
                       "account_id=:account_id AND is_account=:is_account",
                       {{":account_id", accountId}, {":is_account", "false"}})
                    .payloads;
    if (std::find(rows.begin(), rows.end(), profileIds[0]) == rows.end()) {
        insertInto("profiles_accounts",
                   {{":profile_id", "profile_id"},
                    {":account_id", "account_id"},
                    {":is_account", "is_account"}},
                   {{":profile_id", profileIds[0]},
                    {":account_id", accountId},
                    {":is_account", "false"}});
    }
}

} // namespace lrc
