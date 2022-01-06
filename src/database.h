/****************************************************************************
 *   Copyright (C) 2017-2022 Savoir-faire Linux Inc.                        *
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
#pragma once

#include "typedefs.h"

// Qt
#include <QObject>
#include <QtCore/QDir>
#include <QtSql/QSqlQuery>
#include <QtCore/QStandardPaths>
#include <QDebug>

// Std
#include <memory>
#include <string>
#include <stdexcept>
#include <type_traits>

namespace lrc {

static constexpr auto LEGACY_DB_VERSION = "1.1";
static constexpr auto DB_VERSION = "1";

/**
 *  @brief Base class that communicates with a database.
 *  @note not thread safe.
 */
class Database : public QObject
{
    Q_OBJECT

public:
    /**
     * Create a database on the user system.
     * @param the name for which to construct the db.
     * @exception QueryError database query error.
     */
    Database(const QString& name, const QString& basePath);
    ~Database();

    void remove();

    void close();

    virtual void load();

    /**
     * A structure which contains result(s) returned by a database query.
     */
    struct Result
    {
        int nbrOfCols = -1; ///< store the number of columns returned.
        /**
         * if nbrOfCols equals three and if the size of payloads equals six,
         * means two rows of data over three columns.
         */
        VectorString payloads; ///< store the values.
    };

    /**
     * Generic database query exception.
     * details() returns more information, could be empty.
     */
    class QueryError : public std::runtime_error
    {
    public:
        explicit QueryError(QSqlQuery&& query);
        virtual QString details() { return {}; }

        const QSqlQuery query;
    };

    /**
     * Exception on database insert operation.
     * details() returns more information.
     */
    class QueryInsertError final : public QueryError
    {
    public:
        explicit QueryInsertError(QSqlQuery&& query,
                                  const QString& table,
                                  const MapStringString& bindCol,
                                  const MapStringString& bindsSet);
        QString details() override;

        const QString table;
        const MapStringString bindCol;
        const MapStringString bindsSet;
    };

    /**
     * Exception on database update operation.
     * details() returns more information.
     */
    class QueryUpdateError final : public QueryError
    {
    public:
        explicit QueryUpdateError(QSqlQuery&& query,
                                  const QString& table,
                                  const QString& set,
                                  const MapStringString& bindsSet,
                                  const QString& where,
                                  const MapStringString& bindsWhere);
        QString details() override;

        const QString table;
        const QString set;
        const MapStringString bindsSet;
        const QString where;
        const MapStringString bindsWhere;
    };

    /**
     * Exception on database select operation.
     * details() returns more information.
     */
    class QuerySelectError final : public QueryError
    {
    public:
        explicit QuerySelectError(QSqlQuery&& query,
                                  const QString& select,
                                  const QString& table,
                                  const QString& where,
                                  const MapStringString& bindsWhere);
        QString details() override;

        const QString select;
        const QString table;
        const QString where;
        const MapStringString bindsWhere;
    };

    /**
     * Exception on database delete operation.
     * details() returns more information.
     */
    class QueryDeleteError final : public QueryError
    {
    public:
        explicit QueryDeleteError(QSqlQuery&& query,
                                  const QString& table,
                                  const QString& where,
                                  const MapStringString& bindsWhere);
        QString details() override;

        const QString table;
        const QString where;
        const MapStringString bindsWhere;
    };

    /**
     * Exception on database truncate operation.
     * details() returns more information.
     */
    class QueryTruncateError final : public QueryError
    {
    public:
        explicit QueryTruncateError(QSqlQuery&& query, const QString& table);
        QString details() override;

        const QString table;
    };

    /**
     * Insert value(s) inside a table.
     * @param table where to perfom the action on.
     * @param bindCol binds column(s) and identifier(s). The key is the identifier, it should begin
     * by ':'. The value is the name of the column from the table.
     * @param bindsSet binds value(s) and identifier(s). The key is the identifier, it should begin
     * by ':'. The value is the value to store.
     * @return qstring from signed integer representing the index of last inserted element. -1 if
     * nothing inserted.
     * @exception QueryInsertError insert query failed.
     *
     * @note usually the identifiers has to be the same between bindCol and bindsSet
     */
    QString insertInto(const QString& table,
                       const MapStringString& bindCol,
                       const MapStringString& bindsSet);
    /**
     * Update value(s) inside a table.
     * @param table where to perfom the action on.
     * @param set defines which column(s), using identifier(s), will be updated.
     * @param bindsSet specifies the value(s) to set, using the identifier(s). The key is the
     * identifier, it should begin by ':'. The value is value to set.
     * @param where defines the conditional to update, using identifier(s).
     * @param bindsWhere specifies the value(s) to test using the identifier(s). The key is the
     * identifier, it should begin by ':'. The value is the value test.
     * @exception QueryUpdateError update query failed.
     *
     * @note usually, identifiers between set and bindsSet, are equals. The same goes between where
     * and bindsWhere.
     */
    void update(const QString& table,
                const QString& set,
                const MapStringString& bindsSet,
                const QString& where,
                const MapStringString& bindsWhere);
    /**
     * Delete rows from a table.
     * @param table where to perfom the action on.
     * @param where defines the conditional to update, using identifier(s).
     * @param bindsWhere specifies the value(s) to test using the identifier(s). The key is the
     * identifier, it should begin by ':'. The value is the value test.
     * @exception QueryDeleteError delete query failed.
     *
     * @note usually, identifiers between where and bindsWhere, are equals.
     */
    void deleteFrom(const QString& table, const QString& where, const MapStringString& bindsWhere);
    /**
     * Select data from table.
     * @param select column(s) to select.e
     * @param table where to perfom the action on.
     * @param where defines the conditional to select, using identifier(s).
     * @param bindsWhere specifies the value(s) to test using the identifier(s). The key is the
     * identifier, it should begin by ':'. The value is the value to test.
     * @return Database::Result which contains the result(s).
     * @exception QuerySelectError select query failed.
     *
     * @note usually, identifiers between where and bindsWhere, are equals.
     */
    Database::Result select(const QString& select,
                            const QString& table,
                            const QString& where,
                            const MapStringString& bindsWhere);

    /**
     * Returns the count of an expression.
     * @param count is the column to count.
     * @param table where to perfom the action on.
     * @param where defines the conditional to select using identifiers
     * @param bindsWhere specifies the value(s) to test using the identifier(s). The key is the
     * identifier, it should begin by ':'. The value is the value to test.
     */
    int count(const QString& count,
              const QString& table,
              const QString& where,
              const MapStringString& bindsWhere);

    QString basePath_;

protected:
    virtual void createTables();

    /**
     * Migration helpers.
     */
    void migrateIfNeeded();
    void storeVersion(const QString& version);
    QString getVersion();

    virtual void migrateFromVersion(const QString& version);

    QString version_;
    QString connectionName_;
    QString databaseFullPath_;
    QSqlDatabase db_;
};

/**
 *  @brief A legacy database to help migrate from the single db epoch.
 *  @note not thread safe.
 */
class LegacyDatabase final : public Database
{
    Q_OBJECT

public:
    /**
     * Create a migratory legacy database.
     * @exception QueryError database query error.
     */
    LegacyDatabase(const QString& basePath);
    ~LegacyDatabase();

    void load() override;

protected:
    void createTables() override;

private:
    /**
     * Migration helpers from old LRC. Parse JSON for history and VCards and add it into the database.
     */
    void migrateOldFiles();
    void migrateLocalProfiles();
    void migratePeerProfiles();
    void migrateTextHistory();

    void migrateFromVersion(const QString& version) override;

    /**
     * Migration helpers from version 1
     */
    void migrateSchemaFromVersion1();
    void linkRingProfilesWithAccounts(bool contactsOnly);
    void updateProfileAccountForContact(const QString& contactURI, const QString& accountID);
};

namespace DatabaseFactory {
template<typename T, class... Args>
std::enable_if_t<std::is_constructible<T, Args...>::value, std::shared_ptr<Database>>
create(Args&&... args)
{
    auto pdb = std::static_pointer_cast<Database>(std::make_shared<T>(std::forward<Args>(args)...));
    // To allow override of the db load method we don't
    // call it from the constructor.
    try {
        pdb->load();
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(e);
    }
    return pdb;
}
} // namespace DatabaseFactory

} // namespace lrc
