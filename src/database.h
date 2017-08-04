/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
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
#include <string>
#include <memory>
#include <vector>
#include <forward_list>

// Qt
#include <qobject.h>
#include <QtSql/QSqlQuery>


// TEST
#include <qdebug.h>

constexpr char ringDB[] = "ring.db";

//~ const vector RINGDB_TABLES[3][3][10] = {
                                    //~ {"account", 
                                        //~ "id",
                                        //~ "integer primary key"},
                                    //~ {"contacts", 
                                        //~ {"id"                 , "unread"},
                                        //~ {"integer primary key", "integer"}},
                                    //~ {"conversations",
                                        //~ {"id"                 , "from"   , "message"},
                                        //~ {"integer primary key", "integer", "text"}}
                                       //~ }; // Do not forget to update : RINGDB_NUMBER_OF_TABLES
//~ constexpr int RINGDB_NUMBER_OF_TABLES = 3;

/// NOTES :
///   - les messages peuvent contenir des images, soit les traiter comme des blobs soit les mettre ailleurs et les 
///      charger ultérieurement.

class DataBase : public QObject {
    Q_OBJECT
    public:
    ~DataBase();

    void addMessage(const QString& From, const QString& message);
    std::vector<std::string> getMessages(const QString& author);

    //Singleton
    static DataBase& instance();

    // signals
    Q_SIGNALS:
    void messageAdded(const std::string&);

    private:
    explicit DataBase(QObject* parent = nullptr);
    
    std::unique_ptr<QSqlQuery> _querry;
    QSqlDatabase _db;

};
