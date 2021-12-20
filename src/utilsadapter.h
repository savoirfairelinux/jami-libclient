/*
 * Copyright (C) 2015-2020 by Savoir-faire Linux
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 * Author: Isa Nanic <isa.nanic@savoirfairelinux.com>
 * Author: Mingrui Zhang   <mingrui.zhang@savoirfairelinux.com>
 * Author: Aline Gondim Santos   <aline.gondimsantos@savoirfairelinux.com>
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

#include <QApplication>
#include <QObject>

#include "qmladapterbase.h"
#include "appsettingsmanager.h"
#include "qtutils.h"

class QClipboard;
class SystemTray;

#define LOGSLIMIT 10000

class UtilsAdapter final : public QmlAdapterBase
{
    Q_OBJECT
    QML_PROPERTY(QStringList, logList)
public:
    explicit UtilsAdapter(AppSettingsManager* settingsManager,
                          SystemTray* systemTray,
                          LRCInstance* instance,
                          QObject* parent = nullptr);
    ~UtilsAdapter() = default;

    void safeInit() override {}

    Q_INVOKABLE const QString getProjectCredits();
    Q_INVOKABLE const QString getVersionStr();
    Q_INVOKABLE void setClipboardText(QString text);
    Q_INVOKABLE const QString qStringFromFile(const QString& filename);
    Q_INVOKABLE const QString getStyleSheet(const QString& name, const QString& source);
    Q_INVOKABLE const QString getCachePath();
    Q_INVOKABLE bool createStartupLink();
    Q_INVOKABLE QString GetRingtonePath();
    Q_INVOKABLE bool checkStartupLink();
    Q_INVOKABLE void setConversationFilter(const QString& filter);
    Q_INVOKABLE const QString getBestName(const QString& accountId, const QString& uid);
    Q_INVOKABLE const QString getPeerUri(const QString& accountId, const QString& uid);
    Q_INVOKABLE QString getBestId(const QString& accountId);
    Q_INVOKABLE const QString getBestId(const QString& accountId, const QString& uid);
    Q_INVOKABLE const QStringList getCurrAccList();
    Q_INVOKABLE int getAccountListSize();
    Q_INVOKABLE bool hasCall(const QString& accountId);
    Q_INVOKABLE const QString getCallConvForAccount(const QString& accountId);
    Q_INVOKABLE const QString getCallId(const QString& accountId, const QString& convUid);
    Q_INVOKABLE int getCallStatus(const QString& callId);
    Q_INVOKABLE const QString getCallStatusStr(int statusInt);
    Q_INVOKABLE QString getStringUTF8(QString string);
    Q_INVOKABLE bool validateRegNameForm(const QString& regName);
    Q_INVOKABLE QString getRecordQualityString(int value);
    Q_INVOKABLE QString getCurrentPath();
    Q_INVOKABLE QString stringSimplifier(QString input);
    Q_INVOKABLE QString toNativeSeparators(QString inputDir);
    Q_INVOKABLE QString toFileInfoName(QString inputFileName);
    Q_INVOKABLE QString toFileAbsolutepath(QString inputFileName);
    Q_INVOKABLE QString getAbsPath(QString path);
    Q_INVOKABLE QString fileName(const QString& path);
    Q_INVOKABLE QString getExt(const QString& path);
    Q_INVOKABLE bool isImage(const QString& fileExt);
    Q_INVOKABLE QString humanFileSize(qint64 fileSize);
    Q_INVOKABLE void setSystemTrayIconVisible(bool visible);
    Q_INVOKABLE QVariant getAppValue(const Settings::Key key);
    Q_INVOKABLE void setAppValue(const Settings::Key key, const QVariant& value);
    Q_INVOKABLE QString getDirDocument();
    Q_INVOKABLE QString getDirDownload();
    Q_INVOKABLE void setRunOnStartUp(bool state);
    Q_INVOKABLE void setDownloadPath(QString dir);
    Q_INVOKABLE void monitor(const bool& continuous);
    Q_INVOKABLE void clearInteractionsCache(const QString& accountId, const QString& convUid);

Q_SIGNALS:
    void debugMessageReceived(const QString& message);

private:
    QClipboard* clipboard_;
    SystemTray* systemTray_;
    AppSettingsManager* settingsManager_;

    QMetaObject::Connection debugMessageReceivedConnection_;
};
Q_DECLARE_METATYPE(UtilsAdapter*)
