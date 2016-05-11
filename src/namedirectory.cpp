#include "namedirectory.h"

#include <QUrl>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

void NameDirectory::addrLookup(const QString& addr)
{
    qDebug() << "addrLookup " << addr;
    QUrl url("http://" + server_ + addrQuery_ + addr);
    auto reply = net_.get(QNetworkRequest {url});
    connect(reply, &QNetworkReply::finished, this, [this,addr,reply]() {
        reply->deleteLater();
        onAddrLookupReply(addr, reply);
    });
}

void NameDirectory::nameLookup(const QString& name)
{
    qDebug() << "nameLookup " << name;
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
