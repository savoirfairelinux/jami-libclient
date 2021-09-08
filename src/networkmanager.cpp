/*!
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "networkmanager.h"

#include "connectivitymonitor.h"
#include "utils.h"

#include <QMetaEnum>
#include <QtNetwork>

NetWorkManager::NetWorkManager(ConnectivityMonitor* cm, QObject* parent)
    : QObject(parent)
    , manager_(new QNetworkAccessManager(this))
    , reply_(nullptr)
    , connectivityMonitor_(cm)
    , lastConnectionState_(cm->isOnline())
{
    Q_EMIT statusChanged(GetStatus::IDLE);

    connect(connectivityMonitor_, &ConnectivityMonitor::connectivityChanged, [this] {
        cancelRequest();

        auto connected = connectivityMonitor_->isOnline();
        if (connected && !lastConnectionState_) {
            manager_->deleteLater();
            manager_ = new QNetworkAccessManager(this);
            qWarning() << "connectivity changed, reset QNetworkAccessManager";
        }
        lastConnectionState_ = connected;
    });
}

void
NetWorkManager::get(const QUrl& url, const DoneCallBack& doneCb, const QString& path)
{
    if (!connectivityMonitor_->isOnline()) {
        Q_EMIT errorOccured(GetError::DISCONNECTED);
        return;
    }

    if (reply_ && reply_->isRunning()) {
        qWarning() << Q_FUNC_INFO << "currently downloading";
        return;
    } else if (url.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "missing url";
        return;
    }

    if (!path.isEmpty()) {
        QFileInfo fileInfo(url.path());
        QString fileName = fileInfo.fileName();

        file_.reset(new QFile(path + "/" + fileName));
        if (!file_->open(QIODevice::WriteOnly)) {
            Q_EMIT errorOccured(GetError::ACCESS_DENIED);
            file_.reset(nullptr);
            return;
        }
    }

    QNetworkRequest request(url);
    reply_ = manager_->get(request);

    Q_EMIT statusChanged(GetStatus::STARTED);

    connect(reply_,
            QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            [this, doneCb, path](QNetworkReply::NetworkError error) {
                reply_->disconnect();
                reset(true);
                qWarning() << Q_FUNC_INFO << "NetworkError: "
                           << QMetaEnum::fromType<QNetworkReply::NetworkError>().valueToKey(error);
                Q_EMIT errorOccured(GetError::NETWORK_ERROR);
            });

    connect(reply_, &QNetworkReply::finished, [this, doneCb, path]() {
        reply_->disconnect();
        QString response = {};
        if (path.isEmpty())
            response = QString(reply_->readAll());
        reset(!path.isEmpty());
        Q_EMIT statusChanged(GetStatus::FINISHED);
        if (doneCb)
            doneCb(response);
    });

    connect(reply_,
            &QNetworkReply::downloadProgress,
            this,
            &NetWorkManager::downloadProgressChanged);

    connect(reply_, &QNetworkReply::readyRead, this, &NetWorkManager::onHttpReadyRead);

#if QT_CONFIG(ssl)
    connect(reply_,
            SIGNAL(sslErrors(const QList<QSslError>&)),
            this,
            SLOT(onSslErrors(QList<QSslError>)),
            Qt::UniqueConnection);
#endif
}

void
NetWorkManager::reset(bool flush)
{
    reply_->deleteLater();
    reply_ = nullptr;
    if (file_ && flush) {
        file_->flush();
        file_->close();
        file_.reset(nullptr);
    }
}

void
NetWorkManager::onSslErrors(const QList<QSslError>& sslErrors)
{
#if QT_CONFIG(ssl)
    reply_->disconnect();
    reset(true);

    QString errorsString;
    for (const QSslError& error : sslErrors) {
        if (errorsString.length() > 0) {
            errorsString += "\n";
        }
        errorsString += error.errorString();
    }
    Q_EMIT errorOccured(GetError::SSL_ERROR, errorsString);
    return;
#else
    Q_UNUSED(sslErrors);
#endif
}

void
NetWorkManager::onHttpReadyRead()
{
    /*
     * This slot gets called every time the QNetworkReply has new data.
     * We read all of its new data and write it into the file.
     * That way we use less RAM than when reading it at the finished()
     * signal of the QNetworkReply
     */
    if (file_)
        file_->write(reply_->readAll());
}

void
NetWorkManager::cancelRequest()
{
    if (reply_) {
        reply_->abort();
        Q_EMIT errorOccured(GetError::CANCELED);
    }
}
