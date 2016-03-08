/************************************************************************************
 *   Copyright (C) 2016 by Savoir-faire Linux                                       *
 *   Author : Edric Milaret <edric.ladent-milaret@savoirfairelinux.com              *
 *                                                                                  *
 *   This library is free software; you can redistribute it and/or                  *
 *   modify it under the terms of the GNU Lesser General Public                     *
 *   License as published by the Free Software Foundation; either                   *
 *   version 2.1 of the License, or (at your option) any later version.             *
 *                                                                                  *
 *   This library is distributed in the hope that it will be useful,                *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of                 *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU              *
 *   Lesser General Public License for more details.                                *
 *                                                                                  *
 *   You should have received a copy of the GNU Lesser General Public               *
 *   License along with this library; if not, write to the Free Software            *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA *
 ***********************************************************************************/

#include "sqlmanager.h"

#include "itembase.h"

SqlManager::SqlManager() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::DataLocation)
                       + QDir::separator() +"lrc.db3");
    db.open();
    if (not verifySchemaVersion()) {
        //TODO: Handle schema migration somehow
        /*
         * We could use QSqlRecord and some sort of file to describe database
         * then handle migration that way
         */
    }
}

SqlManager::~SqlManager() {
    if (db.isOpen())
        db.close();
}

///Singleton
SqlManager& SqlManager::instance() {
    static auto instance = new SqlManager();

    return *instance;
}

bool SqlManager::isOpen() const {
    return db.isOpen();
}

bool SqlManager::verifySchemaVersion() const {
    if (db.record("_info").isEmpty()) {
        QSqlQuery createInfoTable("CREATE TABLE _info (version INTEGER)");
        createInfoTable.exec();
        QSqlQuery insertDbInfo(QString("INSERT INTO _info (version) VALUES (%1)").arg(version_));
        insertDbInfo.exec();
    } else {
        QSqlQuery schemaVersion("SELECT * FROM _info");
        schemaVersion.exec();
        return (schemaVersion.first() && schemaVersion.value(0).toInt() == version_);
    }
    return true;
}

bool SqlManager::saveItem(const ItemBase& obj) const {
    if (db.isOpen()) {
        auto serialObj = obj.serialize();
        QString field = serialObj.keys().join(", ");
        QStringList valuesList;
        for (auto v : serialObj.keys()) {
            valuesList << ":" + v;
        }
        QSqlQuery query;
        query.prepare(QString("INSERT INTO %1 (%2) VALUES (%3)").arg(obj.getDBName(), field, valuesList.join(", ")));
        for (auto k : serialObj.keys()) {
           query.bindValue(":"+k, serialObj[k]);
        }
        auto ret = query.exec();
        qDebug() << "\n\n\n\n" << query.lastQuery() << "\n\n\n\n";
        if (not ret)
            qWarning() << query.lastError();
        return ret;
    }
    return false;
}
