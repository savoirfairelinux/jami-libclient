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
/* lrc */
#include "namedirectory.h"

/* Qt */
#include <QUrl>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

/* for visual studio */
#include <ciso646>

NameDirectory& NameDirectory::instance()
{
   static auto instance = new NameDirectory;
   return *instance;
}

void NameDirectory::addrLookup(const QString& addr, LookupCallback cb)
{
    auto cacheRes = nameCache_.value(addr);
    if (not cacheRes.isEmpty()) {
        cb(cacheRes, NameDirectory::Response::found);
        return;
    }
    QUrl url("http://" + server_ + addrQuery_ + addr);
    auto reply = net_.get(QNetworkRequest {url});
    connect(reply, &QNetworkReply::finished, this, [this,cb,addr,reply]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            int v = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (v >= 200 && v < 300) // Success
            {
                auto json = QJsonDocument::fromJson(reply->readAll());
                auto name = json.object()["name"].toString();
                if (not name.isEmpty()) {
                    qDebug() << "Found name for" << addr << ":" << name;
                    nameCache_.insert(addr, name);
                    cb(name, NameDirectory::Response::found);
                } else {
                    cb("", NameDirectory::Response::notFound);
                }
            }
        } else {
            cb("", NameDirectory::Response::error);
        }
    });
}

void NameDirectory::nameLookup(const QString& name, LookupCallback cb)
{
    auto cacheRes = addrCache_.value(name);
    if (not cacheRes.isEmpty()) {
        cb(cacheRes, NameDirectory::Response::found);
        return;
    }
    QUrl url("http://" + server_ + nameQuery_ + name);
    auto reply = net_.get(QNetworkRequest {url});
    connect(reply, &QNetworkReply::finished, this, [this,cb,name,reply]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            int v = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (v >= 200 && v < 300) // Success
            {
                auto json = QJsonDocument::fromJson(reply->readAll());
                auto addr = json.object()["addr"].toString();
                if (not name.isEmpty()) {
                    qDebug() << "Found name for" << name << ":" << addr;
                    addrCache_.insert(name, addr);
                    cb(addr, NameDirectory::Response::found);
                }
            }
            cb("", NameDirectory::Response::notFound);
        } else {
            QString error("Network error : "+reply->errorString());
            cb(error, NameDirectory::Response::error);
        }
    });
}