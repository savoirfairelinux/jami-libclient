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

// Parent
#include "database.h"

// Debug
#include <qdebug.h>

// Qt
#include <QtCore/QCoreApplication>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

DataBase::DataBase(QObject* parent)
{
    if (not QSqlDatabase::drivers().contains("QSQLITE")) {
        qDebug() << "DataBase, errror QSQLITE not supported";
        return;
    }

    // initalize the database
    _db = QSqlDatabase::addDatabase("QSQLITE");
    _db.setDatabaseName(ringDB);

    if (not _db.open()) {
        qDebug() << "DataBase, can't open the database";
        return;
    }

    // connect querry object to database object
    // You must load the SQL driver and open the connection before a QSqlQuery is created.
    _querry = std::unique_ptr<QSqlQuery>(new QSqlQuery(_db));

    // check if all tables are presents :
    QStringList tables = _db.tables();

    // add accounts table
    if (not tables.contains("accounts", Qt::CaseInsensitive))
        if (not _querry->exec("create table accounts (id integer primary key)"))
            qDebug() << "DataBase : " << _querry->lastError().text();

    // add contacts table
    if (not tables.contains("contacts", Qt::CaseInsensitive))
        if (not _querry->exec("create table contacts (id integer primary key, unread integer)"))
            qDebug() << "DataBase : " << _querry->lastError().text();

    // add conversations table
    if (not tables.contains("conversations", Qt::CaseInsensitive))
        if (not _querry->exec("create table conversations (id integer primary key, author integer, message text)"))
            qDebug() << "DataBase : " << _querry->lastError().text();

}

DataBase::~DataBase()
{
}

DataBase& DataBase::instance()
{
    static auto instance = new DataBase(QCoreApplication::instance());
    return *instance;
}

void
DataBase::addMessage(const QString& author, const QString& message )
{
    auto toto = QString("insert into conversations(author, message) values(? , ?)");

    if (not _querry->prepare(toto)) {
        qDebug() << "addMessage, " << _querry->lastError().text();
        return;
    }

    _querry->addBindValue(author);
    _querry->addBindValue(message);

    if (not _querry->exec()) {
        qDebug() << "addMessage, " << _querry->lastError().text();
        return;
    }

    emit messageAdded(message.toStdString()); // ajouter l'auteur
}

std::vector<std::string> //  message : status+date+contactitem+direction+body
DataBase::getMessages(const QString& author) // author == from
{
    // ici je recupere le hash du compte
    // je fais la requete pour avoir les messages du couple account+contact
    auto toto = QString("select message from conversations where author = '"+author+"'");

    if (not _querry->exec(toto)) {
        qDebug() << "getMessages, " << _querry->lastError().text();
        return std::vector<std::string>();
    }

    std::vector<std::string> messages;    
    while(_querry->next())
        messages.push_back(_querry->value(0).toString().toStdString());

    return messages;
}


#include <database.moc>
