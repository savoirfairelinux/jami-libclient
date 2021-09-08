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

#include "updatemanager.h"

#include "lrcinstance.h"
#include "utils.h"
#include "version.h"

#include <QProcess>
#include <QTimer>

#ifdef BETA
static constexpr bool isBeta = true;
#else
static constexpr bool isBeta = false;
#endif

static constexpr int updatePeriod = 1000 * 60 * 60 * 24; // one day in millis
static constexpr char downloadUrl[] = "https://dl.jami.net/windows";
static constexpr char versionSubUrl[] = "/version";
static constexpr char betaVersionSubUrl[] = "/beta/version";
static constexpr char msiSubUrl[] = "/jami.release.x64.msi";
static constexpr char betaMsiSubUrl[] = "/beta/jami.beta.x64.msi";

UpdateManager::UpdateManager(const QString& url,
                             ConnectivityMonitor* cm,
                             LRCInstance* instance,
                             QObject* parent)
    : NetWorkManager(cm, parent)
    , lrcInstance_(instance)
    , baseUrlString_(url.isEmpty() ? downloadUrl : url)
    , tempPath_(Utils::WinGetEnv("TEMP"))
    , updateTimer_(new QTimer(this))
{
    connect(updateTimer_, &QTimer::timeout, [this] {
        // Quiet period update check.
        checkForUpdates(true);
    });
}

void
UpdateManager::setAutoUpdateCheck(bool state)
{
    // Quiet check for updates periodically, if set to.
    if (!state) {
        updateTimer_->stop();
        return;
    }
    updateTimer_->start(updatePeriod);
}

bool
UpdateManager::isCurrentVersionBeta()
{
    return isBeta;
}

void
UpdateManager::checkForUpdates(bool quiet)
{
    disconnect();

    // Fail without UI if this is a programmatic check.
    if (!quiet)
        connect(this, &NetWorkManager::errorOccured, this, &UpdateManager::updateCheckErrorOccurred);

    cleanUpdateFiles();
    QUrl versionUrl {isBeta ? QUrl::fromUserInput(baseUrlString_ + betaVersionSubUrl)
                            : QUrl::fromUserInput(baseUrlString_ + versionSubUrl)};
    get(versionUrl, [this, quiet](const QString& latestVersionString) {
        if (latestVersionString.isEmpty()) {
            qWarning() << "Error checking version";
            if (!quiet)
                Q_EMIT updateCheckReplyReceived(false);
            return;
        }
        auto currentVersion = QString(VERSION_STRING).toULongLong();
        auto latestVersion = latestVersionString.toULongLong();
        qDebug() << "latest: " << latestVersion << " current: " << currentVersion;
        if (latestVersion > currentVersion) {
            qDebug() << "New version found";
            Q_EMIT updateCheckReplyReceived(true, true);
        } else {
            qDebug() << "No new version found";
            if (!quiet)
                Q_EMIT updateCheckReplyReceived(true, false);
        }
    });
}

void
UpdateManager::applyUpdates(bool beta)
{
    disconnect();
    connect(this, &NetWorkManager::errorOccured, this, &UpdateManager::updateDownloadErrorOccurred);
    connect(this, &NetWorkManager::statusChanged, [this](GetStatus status) {
        switch (status) {
        case GetStatus::STARTED:
            connect(this,
                    &NetWorkManager::downloadProgressChanged,
                    this,
                    &UpdateManager::updateDownloadProgressChanged);
            Q_EMIT updateDownloadStarted();
            break;
        case GetStatus::FINISHED:
            Q_EMIT updateDownloadFinished();
            break;
        default:
            break;
        }
    });

    QUrl downloadUrl {(beta || isBeta) ? QUrl::fromUserInput(baseUrlString_ + betaMsiSubUrl)
                                       : QUrl::fromUserInput(baseUrlString_ + msiSubUrl)};

    get(
        downloadUrl,
        [this, downloadUrl](const QString&) {
            lrcInstance_->finish();
            Q_EMIT lrcInstance_->quitEngineRequested();
            auto args = QString(" /passive /norestart WIXNONUILAUNCH=1");
            QProcess process;
            process.start("powershell ",
                          QStringList() << tempPath_ + "\\" + downloadUrl.fileName() << "/L*V"
                                        << tempPath_ + "\\jami_x64_install.log" + args);
            process.waitForFinished();
        },
        tempPath_);
}

void
UpdateManager::cancelUpdate()
{
    cancelRequest();
}

void
UpdateManager::cleanUpdateFiles()
{
    /*
     * Delete all logs and msi in the %TEMP% directory before launching.
     */
    QString dir = QString(Utils::WinGetEnv("TEMP"));
    QDir log_dir(dir, {"jami*.log"});
    for (const QString& filename : log_dir.entryList()) {
        log_dir.remove(filename);
    }
    QDir msi_dir(dir, {"jami*.msi"});
    for (const QString& filename : msi_dir.entryList()) {
        msi_dir.remove(filename);
    }
    QDir version_dir(dir, {"version"});
    for (const QString& filename : version_dir.entryList()) {
        version_dir.remove(filename);
    }
}
