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
#include "namedirectory.h"

#include <QUrl>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

void NameDirectory::addrLookup(const QString& addr)
{
    QUrl url("http://" + server_ + addrQuery_ + addr);
    auto reply = net_.get(QNetworkRequest {url});
    connect(reply, &QNetworkReply::finished, this, [this,addr,reply]() {
        reply->deleteLater();
        onAddrLookupReply(addr, reply);
    });
}

void NameDirectory::nameLookup(const QString& name)
{
    QUrl url("http://" + server_ + nameQuery_ + name);
    auto reply = net_.get(QNetworkRequest {url});
    connect(reply, &QNetworkReply::finished, this, [this,name,reply]() {
        reply->deleteLater();
        onNameLookupReply(name, reply);
    });
}

void NameDirectory::onAddrLookupReply(const QString& addr, QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        int v = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (v >= 200 && v < 300) // Success
        {
            auto json = QJsonDocument::fromJson(reply->readAll());
            auto name = json.object()["name"].toString();
            if (not name.isEmpty()) {
                qDebug() << "Found name for" << addr << ":" << name;
                emit addrFound(addr, name);
            }
        }
    }
}

void NameDirectory::onNameLookupReply(const QString& name, QNetworkReply* reply)
{
    reply->deleteLater();
    if (reply->error() == QNetworkReply::NoError) {
        int v = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (v >= 200 && v < 300) // Success
        {
            auto json = QJsonDocument::fromJson(reply->readAll());
            auto addr = json.object()["addr"].toString();
            if (not name.isEmpty()) {
                qDebug() << "Found address for" << name << ":" << addr;
                emit nameFound(name, addr);
            }
        }
    }
}

NameDirectory& NameDirectory::instance()
{
   static auto instance = new NameDirectory;
   return *instance;
}
