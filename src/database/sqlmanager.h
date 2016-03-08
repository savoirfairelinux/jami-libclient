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

#pragma once

#include <QtCore/QObject>
#include <QtSql/QtSql>

class ItemBase;

class SqlManager : QObject {
    Q_OBJECT
public:
    static SqlManager& instance();
    bool isOpen() const;

    template<typename T>
    bool saveItem(const ItemBase& obj) const;
    template<typename T>
    QList<QMap<QString, QVariant>> loadItems() const;
    template<typename T>
    bool deleteAll() const;
    template<typename T>
    bool deleteItem(const T* obj) const;

private:
    SqlManager();
    ~SqlManager();
    bool verifySchemaVersion() const;

    template<typename T>
    QString getTableName() const;

    QSqlDatabase db;
    constexpr static const int version_ { 1 };
};

#include "sqlmanager.hpp"
