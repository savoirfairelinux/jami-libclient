/****************************************************************************
 *   Copyright (C) 2016 by Savoir-faire Linux                          *
 *   Author : Adrien Béraud <adrien.beraud@savoirfairelinux.com> *
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

class NameDirectory : public QObject
{
    Q_OBJECT

public:
    NameDirectory() : net_(this) {
        //connect(&net_, SIGNAL(finished(QNetworkReply *)), this, SLOT(onNameLookupReply(QNetworkReply *)));
        //connect(&net_, SIGNAL(finished(QNetworkReply *)), this, SLOT(onNameLookupReply(QNetworkReply *)));
    }

    static NameDirectory& instance();

public slots:
    void nameLookup(const QString& name);
    void addrLookup(const QString& addr);

signals:
    void nameFound(const QString& name, const QString& addr);
    void addrFound(const QString& addr, const QString& name);

private:
    struct Record {
        URI uri;
        //QString owner;
    };

    QString server_ {"localhost:3000"};
    const QString nameQuery_ {"/name/"}; 
    const QString addrQuery_ {"/addr/"}; 
    QNetworkAccessManager net_;
    QMap<QString, Record> cache_;

private slots:
    void onNameLookupReply(const QString& name, QNetworkReply*);
    void onAddrLookupReply(const QString& addr, QNetworkReply*);

};
