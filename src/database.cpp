/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>          *
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
#include <QtSql/QSqlRecord>
#include <QStandardPaths>
#include <QDebug>

// Data
#include "api/message.h"

namespace lrc
{

using namespace api;

static constexpr auto VERSION = "1";
static constexpr auto NAME = "ring.db";

Database::Database()
: QObject()
{
    // check support.
    if (not QSqlDatabase::drivers().contains("QSQLITE")) {
        qDebug() << "Database, error QSQLITE not supported";
        throw Errors::QSQLITE_NOT_SUPPORTED;
    }

    // initalize the database.
    db_ = QSqlDatabase::addDatabase("QSQLITE");
    db_.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + NAME);

    // open the database.
    if (not db_.open()) {
        qDebug() << "Database, can't open the database";
        throw Errors::CANNOT_OPEN_DB;
    }

    // if db is empty we create them.
    if (db_.tables().empty())
        createTables();
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
                                                 type INTEGER,          \
                                                 status INTEGER)";

    auto tableConversations = "CREATE TABLE conversations (id INTEGER PRIMARY KEY,\
                                                           participant_id INTEGER, \
                                                           FOREIGN KEY(participant_id) REFERENCES profiles(id))";

    auto tableInteractions = "CREATE TABLE interactions (id INTEGER PRIMARY KEY,\
                                                         account_id INTEGER, \
                                                         author_id INTEGER, \
                                                         conversation_id INTEGER, \
                                                         device_id INTEGER, \
                                                         group_id INTEGER, \
                                                         timestamp INTEGER, \
                                                         body TEXT,     \
                                                         type INTEGER,  \
                                                         status INTEGER, \
                                                         FOREIGN KEY(account_id) REFERENCES profiles(id), \
                                                         FOREIGN KEY(author_id) REFERENCES profiles(id), \
                                                         FOREIGN KEY(conversation_id) REFERENCES conversations(id))";
    // add profiles table
    if (not db_.tables().contains("profiles", Qt::CaseInsensitive)
        and not query.exec(tableProfiles)) {
            qDebug() << "Database: " << query.lastError().text();
            throw Errors::QUERY_FAILED;
    }

    // add conversations table
    if (not db_.tables().contains("conversations", Qt::CaseInsensitive)
        and not query.exec(tableConversations)) {
            qDebug() << "Database: " << query.lastError().text();
            throw Errors::QUERY_FAILED;
    }

    // add interactions table
    if (not db_.tables().contains("interactions", Qt::CaseInsensitive)
        and not query.exec(tableInteractions)) {
            qDebug() << "Database: " << query.lastError().text();
            throw Errors::QUERY_FAILED;
    }

    return storeVersion(VERSION);
}

void
Database::storeVersion(const std::string& version)
{
    QSqlQuery query;

    auto storeVersionQuery = std::string("PRAGMA user_version = ") + version;

    if (not query.exec(storeVersionQuery.c_str())) {
        qDebug() << "Database: " << query.lastError().text();
        throw Errors::QUERY_FAILED;
    }
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

    if (not query.exec()) {
        qDebug() << "sqlite error: " << query.lastError();
        qDebug() << "paramaters sent :";
        qDebug() << "table = " << table.c_str();
        for (auto& b : bindCol)
            qDebug() << "   {" << b.first.c_str() << "}, {" << b.second.c_str() <<"}";
        for (auto& b : bindsSet)
            qDebug() << "   {" << b.first.c_str() << "}, {" << b.second.c_str() <<"}";
        throw Errors::QUERY_FAILED;
    }

    if (not query.exec("SELECT last_insert_rowid()")) {
        qDebug() << "sqlite error: " << query.lastError();
        throw Errors::QUERY_FAILED;
    }

    if (!query.next())
        throw Errors::QUERY_FAILED;

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

    if (not query.exec()) {
        qDebug() << "sqlite error: " << query.lastError();
        qDebug() << "paramaters sent :";
        qDebug() << "table = " << table.c_str();
        qDebug() << "set = " << set.c_str();
        qDebug() << "bindsSet :";
        for (auto& b : bindsSet)
            qDebug() << "   {" << b.first.c_str() << "}, {" << b.second.c_str() <<"}";
        qDebug() << "where = " << where.c_str();
        qDebug() << "bindsWhere :";
        for (auto& b : bindsWhere)
            qDebug() << "   {" << b.first.c_str() << "}, {" << b.second.c_str() <<"}";
        throw Errors::QUERY_FAILED;
    }
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

    if (not query.exec()) {
        qDebug() << "sqlite error: " << query.lastError();
        qDebug() << "paramaters sent :";
        qDebug() << "select :" << select.c_str();
        qDebug() << "table = " << table.c_str();
        qDebug() << "where = " << where.c_str();
        qDebug() << "bindsWhere :";
        for (auto& b : bindsWhere)
            qDebug() << "   {" << b.first.c_str() << "}, {" << b.second.c_str() <<"}";
        throw Errors::QUERY_FAILED;
    }

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

    if(not query.exec()) {
        qDebug() << "sqlite error: " << query.lastError();
        qDebug() << "paramaters sent :";
        qDebug() << "table = " << table.c_str();
        qDebug() << "where = " << where.c_str();
        qDebug() << "bindsWhere :";
        for (auto& b : bindsWhere)
            qDebug() << "   {" << b.first.c_str() << "}, {" << b.second.c_str() <<"}";
        throw Errors::QUERY_FAILED;
    }
}

} // namespace lrc
