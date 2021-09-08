/*!
 * Copyright (C) 2020 by Savoir-faire Linux
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

#include "networkmanager.h"

class LRCInstance;
class ConnectivityMonitor;
class QTimer;

class UpdateManager final : public NetWorkManager
{
    Q_OBJECT
public:
    explicit UpdateManager(const QString& url,
                           ConnectivityMonitor* cm,
                           LRCInstance* instance = nullptr,
                           QObject* parent = nullptr);
    ~UpdateManager() = default;

    Q_INVOKABLE void checkForUpdates(bool quiet = false);
    Q_INVOKABLE void applyUpdates(bool beta = false);
    Q_INVOKABLE void cancelUpdate();
    Q_INVOKABLE void setAutoUpdateCheck(bool state);
    Q_INVOKABLE bool isCurrentVersionBeta();

Q_SIGNALS:
    void updateCheckReplyReceived(bool ok, bool found = false);
    void updateCheckErrorOccurred(GetError error);
    void updateDownloadStarted();
    void updateDownloadProgressChanged(qint64 bytesRead, qint64 totalBytes);
    void updateDownloadErrorOccurred(GetError error);
    void updateDownloadFinished();
    void appCloseRequested();

private:
    // LRCInstance pointer
    LRCInstance* lrcInstance_ {nullptr};

    QString baseUrlString_;
    QString tempPath_;
    QTimer* updateTimer_;

    void cleanUpdateFiles();
};
Q_DECLARE_METATYPE(UpdateManager*)
