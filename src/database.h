/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
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
#pragma once

// Std
#include <memory>
#include <string>
#include <stdexcept>

// Qt
#include <qobject.h>
#include <QtSql/QSqlQuery>

namespace lrc
{

/**
  *  @brief Class that communicates with the database.
  *  @note not thread safe.
  */
class Database : public QObject {
    Q_OBJECT

public:
    /**
     * Create a database on the user system.
     * @exception QueryError database query error.
     */
    Database();

    ~Database();

    /**
     * A structure wich contains result(s) returned by a database query.
     */
    struct Result {
        int nbrOfCols = -1; ///< store the number of columns returned.
        /**
         * if nbrOfCols equals three and if the size of payloads equals six,
         * means two rows of data over three columns.
         */
        std::vector<std::string> payloads; ///< store the values.
    };

    /**
     * Generic database query exception.
     * details() returns more information, could be empty.
     */
    class QueryError : public std::runtime_error {
    public:
        explicit QueryError(const QSqlQuery& query);
        virtual std::string details() { return {}; }

        const QSqlQuery query;
    };

    /**
     * Exception on database insert operation.
     * details() returns more information.
     */
    class QueryInsertError final : public QueryError {
    public:
        explicit QueryInsertError(const QSqlQuery& query,
                                  const std::string& table,
                                  const std::map<std::string, std::string>& bindCol,
                                  const std::map<std::string, std::string>& bindsSet);
        std::string details() override;

        const std::string table;
        const std::map<std::string, std::string> bindCol;
        const std::map<std::string, std::string> bindsSet;
    };

    /**
     * Exception on database update operation.
     * details() returns more information.
     */
    class QueryUpdateError final : public QueryError {
    public:
        explicit QueryUpdateError(const QSqlQuery& query,
                                  const std::string& table,
                                  const std::string& set,
                                  const std::map<std::string, std::string>& bindsSet,
                                  const std::string& where,
                                  const std::map<std::string, std::string>& bindsWhere);
        std::string details() override;

        const std::string table;
        const std::string set;
        const std::map<std::string, std::string> bindsSet;
        const std::string where;
        const std::map<std::string, std::string> bindsWhere;
    };

    /**
     * Exception on database select operation.
     * details() returns more information.
     */
    class QuerySelectError final : public QueryError {
    public:
        explicit QuerySelectError(const QSqlQuery& query,
                                  const std::string& select,
                                  const std::string& table,
                                  const std::string& where,
                                  const std::map<std::string, std::string>& bindsWhere);
        std::string details() override;

        const std::string select;
        const std::string table;
        const std::string where;
        const std::map<std::string, std::string> bindsWhere;
    };

    /**
     * Exception on database delete operation.
     * details() returns more information.
     */
    class QueryDeleteError final : public QueryError {
    public:
        explicit QueryDeleteError(const QSqlQuery& query,
                                  const std::string& table,
                                  const std::string& where,
                                  const std::map<std::string, std::string>& bindsWhere);
        std::string details() override;

        const std::string table;
        const std::string where;
        const std::map<std::string, std::string> bindsWhere;
    };

    /**
     * Insert value(s) inside a table.
     * @param table where to perfom the action on.
     * @param bindCol binds column(s) and identifier(s). The key is the identifier, it should begin by ':'.
     *        The value is the name of the column from the table.
     * @param bindsSet binds value(s) and identifier(s). The key is the identifier, it should begin by ':'.
     *        The value is the value to store.
     * @return signed integer representing the index of last inserted element. -1 if nothing inserted.
     * @exception QueryInsertError insert query failed.
     *
     * @note usually the identifiers has to be the same between bindCol and bindsSet
     */
    int insertInto(const std::string& table,
                   const std::map<std::string, std::string>& bindCol,
                   const std::map<std::string, std::string>& bindsSet);
    /**
     * Update value(s) inside a table.
     * @param table where to perfom the action on.
     * @param set defines which column(s), using identifier(s), will be updated.
     * @param bindsSet specifies the value(s) to set, using the identifier(s). The key is the identifier, it should
     *        begin by ':'. The value is value to set.
     * @param where defines the conditional to update, using identifier(s).
     * @param bindsWhere specifies the value(s) to test using the identifier(s). The key is the identifier, it should
     *        begin by ':'. The value is the value test.
     * @exception QueryUpdateError update query failed.
     *
     * @note usually, identifiers between set and bindsSet, are equals. The same goes between where and bindsWhere.
     */
    void update(const std::string& table,
                const std::string& set,
                const std::map<std::string, std::string>& bindsSet,
                const std::string& where,
                const std::map<std::string, std::string>& bindsWhere);
    /**
     * Delete rows from a table.
     * @param table where to perfom the action on.
     * @param where defines the conditional to update, using identifier(s).
     * @param bindsWhere specifies the value(s) to test using the identifier(s). The key is the identifier, it should
     *        begin by ':'. The value is the value test.
     * @exception QueryDeleteError delete query failed.
     *
     * @note usually, identifiers between where and bindsWhere, are equals.
     */
    void deleteFrom(const std::string& table,
                    const std::string& where,
                    const std::map<std::string, std::string>& bindsWhere);
    /**
     * Select data from table.
     * @param select column(s) to select.e
     * @param table where to perfom the action on.
     * @param where defines the conditional to select, using identifier(s).
     * @param bindsWhere specifies the value(s) to test using the identifier(s). The key is the identifier, it should
     *        begin by ':'. The value is the value to test.
     * @return Database::Result which contains the result(s).
     * @exception QuerySelectError select query failed.
     *
     * @note usually, identifiers between where and bindsWhere, are equals.
     */
    Database::Result select(const std::string& select,
                            const std::string& table,
                            const std::string& where,
                            const std::map<std::string, std::string>& bindsWhere);

private:
    void createTables();
    void storeVersion(const std::string& version);

    /**
     * Migration helpets. Parse JSON for history and VCards and add it into the database.
     */
    void migrateOldFiles();
    void migrateLocalProfiles();
    void migratePeerProfiles();
    void migrateTextHistory();

    QSqlDatabase db_;
};

} // namespace lrc
