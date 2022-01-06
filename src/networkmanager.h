/*!
 * Copyright (C) 2019-2022 Savoir-faire Linux Inc.
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

#pragma once

#include <QObject>
#include <QFile>
#include <QSslError>
#include <QNetworkReply>

class QNetworkAccessManager;
class ConnectivityMonitor;

class NetWorkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetWorkManager(ConnectivityMonitor* cm, QObject* parent = nullptr);
    virtual ~NetWorkManager() = default;

    enum GetStatus { IDLE, STARTED, FINISHED };

    enum GetError { DISCONNECTED, NETWORK_ERROR, ACCESS_DENIED, SSL_ERROR, CANCELED };
    Q_ENUM(GetError)

    using DoneCallBack = std::function<void(const QString&)>;

    /*!
     * using qt get request to store the reply in file
     * @param url - network address
     * @param doneCb - done callback
     * @param path - optional file saving path, if empty
     * a string will be passed as the second paramter of doneCb
     */
    void get(const QUrl& url, const DoneCallBack& doneCb = {}, const QString& path = {});

    /*!
     * manually abort the current request
     */
    Q_INVOKABLE void cancelRequest();

Q_SIGNALS:
    void statusChanged(GetStatus error);
    void downloadProgressChanged(qint64 bytesRead, qint64 totalBytes);
    void errorOccured(GetError error, const QString& msg = {});

private Q_SLOTS:
    void onSslErrors(const QList<QSslError>& sslErrors);
    void onHttpReadyRead();

private:
    void reset(bool flush = true);

    QNetworkAccessManager* manager_;
    QNetworkReply* reply_;
    QScopedPointer<QFile> file_;
    ConnectivityMonitor* connectivityMonitor_;
    bool lastConnectionState_;
};
Q_DECLARE_METATYPE(NetWorkManager*)
