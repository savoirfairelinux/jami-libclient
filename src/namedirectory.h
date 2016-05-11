/****************************************************************************
 *   Copyright (C) 2016 by Savoir-faire Linux                               *
 *   Author : Adrien Béraud <adrien.beraud@savoirfairelinux.com>            *
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

#include "uri.h"

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>

#include <functional>

class NameDirectory : public QObject
{
    Q_OBJECT

public:
    NameDirectory() : net_(this) {}
    static NameDirectory& instance();

    enum Response{ found, notFound, error };

    using LookupCallback = std::function<void(const QString& result, const Response& response)>;
    void addrLookup(const QString& addr, LookupCallback cb);
    void nameLookup(const QString& name, LookupCallback cb);

    void setServer(QString server) {
        server_ = server;
    }

private:
    QString server_ {"localhost:3000"};
    const QString nameQuery_ {"/name/"};
    const QString addrQuery_ {"/addr/"};
    QNetworkAccessManager net_;
    QMap<QString, QString> nameCache_;
    QMap<QString, QString> addrCache_;
};
