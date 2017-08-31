/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author:  Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
 *   Author:  Sébastien Blin <sebastien.blin@savoirfairelinux.com>          *
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

// Qt
#include <qobject.h>
#include <QtSql/QSqlQuery>

namespace lrc
{

class Database : public QObject {
    Q_OBJECT

public:
    Database();
    ~Database();
    /**
     * A structure wich contains the result(s) returned by a query to the database.
     * @note nbrOfCols store the number of column returned.
     * @note payloads store the values. if nbrOfCols equal three and if the size of payloads equal six, means two row of
     * data over three columns.
     */
    struct Result {
        int nbrOfCols = -1;
        std::vector<std::string> payloads;
    };
    /**
     * Insert value(s) inside a table.
     * @param table where to perfom the action on.
     * @param bindCol binds column(s) and identifier(s). The key is the identifier, it should begin by ':'.
     *        The value is the name of the column from the table.
     * @param bindsSet binds value(s) and identifier(s). The key is the identifier, it should begin by ':'.
     *        The value is the value to store.
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
     *
     * @note usually, identifiers between set and bindsSet, are equals. The same goes between where and bindsWhere.
     */
    bool update(const std::string& table,
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
     *
     * @note usually, identifiers between where and bindsWhere, are equals.
     */
    bool deleteFrom(const std::string& table,
                    const std::string& where,
                    const std::map<std::string, std::string>& bindsWhere);
    /**
     * Select data from table.
     * @param select, column(s) to select.
     * @param table where to perfom the action on.
     * @param where defines the conditional to select, using identifier(s).
     * @param bindsWhere specifies the value(s) to test using the identifier(s). The key is the identifier, it should
     *        begin by ':'. The value is the value to test.
     * @return Database::Result which contains the result(s).
     *
     * @note usually, identifiers between where and bindsWhere, are equals.
     */
    Database::Result select(const std::string& select,
                            const std::string& table,
                            const std::string& where,
                            const std::map<std::string, std::string>& bindsWhere);

private:
    bool createTables();
    bool storeVersion(const std::string& version);
    static constexpr auto VERSION = "1";
    static constexpr auto NAME = "ring.db";
    QSqlDatabase db_;
};

} // namespace lrc
