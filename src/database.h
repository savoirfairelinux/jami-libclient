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
    static constexpr auto ringDB = "ring.db";

    Database();
    ~Database();
    
    struct Result {
        int nbrOfCols = -1;
        std::vector<std::string> payloads;
    };

    bool createTables();

    bool storeVersion(const std::string& version);

    bool insertInto(const std::string& table,
                    const std::map<std::string, std::string>& bindCol,
                    const std::map<std::string, std::string>& bindsSet) const;

    bool update(const std::string& table,
                const std::string& set,
                const std::map<std::string, std::string>& bindsSet,
                const std::string& where,
                const std::map<std::string, std::string>& bindsWhere) const;

    bool deleteFrom(const std::string& table,
                    const std::string& where,
                    const std::map<std::string, std::string>& binds) const;

    Database::Result select(const std::string& select,
                            const std::string& table,
                            const std::string& where,
                            const std::map<std::string, std::string>& binds) const;

private:
    const std::string VERSION="1";
    QSqlDatabase db_;
};

} // namespace lrc
