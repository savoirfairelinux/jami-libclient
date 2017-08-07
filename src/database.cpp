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
    qDebug() << "{C} DataBase" ;
    if (not QSqlDatabase::drivers().contains("QSQLITE")) {
        qDebug() << "Error";
        // do something or leave...
    }

    // initalize the database

    _db = QSqlDatabase::addDatabase("QSQLITE");//not dbConnection
    _db.setDatabaseName(ringDB);

    if (not _db.open()) {
        qDebug() << "Error";
        return;
        // do something or leave...
    }

    // connect query object to database object
    // You must load the SQL driver and open the connection before a QSqlQuery is created.
    _query = std::unique_ptr<QSqlQuery>(new QSqlQuery(_db));

    // check if all tables are presents :
    QStringList tables = _db.tables();

    // add accounts table
    if (not tables.contains("accounts", Qt::CaseInsensitive))
        if (not _query->exec("create table accounts (id integer primary key)"))
            qDebug() << "DataBase : " << _query->lastError().text();

    // add contacts table
    if (not tables.contains("contacts", Qt::CaseInsensitive))
        if (not _query->exec("create table contacts (id integer primary key, unread integer)"))
            qDebug() << "DataBase : " << _query->lastError().text();

    // add conversations table
    if (not tables.contains("conversations", Qt::CaseInsensitive))
        if (not _query->exec("create table conversations (id integer primary key, author integer, message text, timestamp text)"))
            qDebug() << "DataBase : " << _query->lastError().text();

    // fake message
    addMessage("2518b726b225019595a09ab6735785b3384f99a4", "hello!", "1501474286");

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
DataBase::addMessage(const QString& author, const QString& message, const QString& timestamp)
{
    qDebug() << "G: " << author;
    qDebug() << "G: " << message;

    auto toto = QString("insert into conversations(author, message, timestamp) values(? , ? , ?)");

    if (not _query->prepare(toto)) {
        qDebug() << "addMessage, " << _query->lastError().text();
        return;
    }

    _query->addBindValue(author);
    _query->addBindValue(message);
    _query->addBindValue(timestamp);

    if (not _query->exec()) {
        qDebug() << "addMessage, " << _query->lastError().text();
        return;
    }

    DataBase::Message msg;
    msg.body = message.toUtf8().constData();
    msg.timestamp = timestamp.toStdString();

    emit messageAdded(msg); // ajouter l'auteur
}

std::vector<DataBase::Message> //  message : status+date+contactitem+direction+body
DataBase::getMessages(const QString& author) // author == from
{
    // ici je recupere le hash du compte
    // je fais la requete pour avoir les messages du couple account+contact
    auto toto = QString("SELECT message, timestamp FROM conversations WHERE author = '"+author+"'");

    if (not _query->exec(toto)) {
        qDebug() << "getMessages, " << _query->lastError().text();
        return std::vector<Message>();
    }

    std::vector<DataBase::Message> messages;
    while(_query->next()) {
        DataBase::Message msg;
        msg.body = _query->value(0).toString().toStdString();
        msg.timestamp = _query->value(1).toString().toStdString();
        messages.push_back(msg);
    }

    return messages;
}


#include <database.moc>
