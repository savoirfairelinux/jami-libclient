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
#include "sqlmigratorhelper.h"

SqlManager::SqlManager() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    auto dbFilePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation)
            + QDir::separator();
    db.setDatabaseName(dbFilePath + "lrc.db3");
    db.open();
    if (not verifySchemaVersion()) {
        auto noError = true;

        //0 - Open logfile
        QFile logFile(QString("%1migrationFromV%2.log").arg(dbFilePath).arg(actualVersion_));
        logFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
        QTextStream logStream(&logFile);

        //1 - Backup the old database
        logStream << "Starting DB backup" << endl;
        noError = QFile::copy(dbFilePath + "lrc.db3",
                    dbFilePath + QString("lrc_backupFromV%1.db3").arg(actualVersion_));
        if (noError) {
            logStream << "DB backup complete" << endl;
            //2 - Migrate going trough modification by one version until done
            SqlMigratorHelper helper;
            for (int i = actualVersion_; i < version_ && noError; i++) {
                //MIGRATE
                logStream << "Starting migration from "  << actualVersion_ << " to " << i + 1 << endl;
                auto instructions = helper.getMigrateInstructions(i + 1);
                for (auto instr : instructions) {
                    QSqlQuery migrateInstru(instr);
                    logStream << migrateInstru.lastQuery() << endl;
                    noError = migrateInstru.exec();
                    if (not noError) {
                        logStream << migrateInstru.lastError().text() << endl;
                        break;
                    }
                }
            }
        }
        //3a - If it worked, we re good supress backup
        if (noError) {
            logStream << "Updating new schema version number" << endl;
            //setSchemaVersion(version_);
            logStream << "Schema version number updated" << endl;
            logStream << "Migration complete removing backup" << endl;
            QFile::remove(dbFilePath + QString("lrc_backupFromV%1.db3").arg(actualVersion_));
            logStream << "Backup removed" << endl;
            logStream << "Migration Success" << endl;

        } else { //3b - Else we re in deep shit
            db.close();
            logStream << "Removing corrupted database" << endl;
            QFile::remove(dbFilePath + QString("lrc.db3"));
            logStream << "Reinstating old database" << endl;
            QFile::rename(dbFilePath + QString("lrc_backupFromV%1.db3").arg(actualVersion_),
                          dbFilePath + QString("lrc.db3"));
            logStream << "Migration Failed" << endl;
            throw std::runtime_error("Database could not be migrated");
        }

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

bool SqlManager::verifySchemaVersion() {
    QSqlQuery schemaVersion("PRAGMA user_version");
    schemaVersion.exec();
    if (not schemaVersion.first())
        return false;
    actualVersion_ = schemaVersion.value(0).toInt();
    return version_ == actualVersion_;
}

bool SqlManager::setSchemaVersion(int version) const {
    QSqlQuery setVersionQuery(QString("PRAGMA user_version = %1").arg(version));
    return setVersionQuery.exec();
}
