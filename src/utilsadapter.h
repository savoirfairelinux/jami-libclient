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

class QClipboard;

class UtilsAdapter final : public QObject
{
    Q_OBJECT
public:
    explicit UtilsAdapter(QObject* parent = nullptr);
    ~UtilsAdapter() = default;

    Q_INVOKABLE const QString getChangeLog();
    Q_INVOKABLE const QString getProjectCredits();
    Q_INVOKABLE const QString getVersionStr();
    Q_INVOKABLE void setText(QString text);
    Q_INVOKABLE const QString qStringFromFile(const QString& filename);
    Q_INVOKABLE const QString getStyleSheet(const QString& name, const QString& source);
    Q_INVOKABLE const QString getCachePath();
    Q_INVOKABLE bool createStartupLink();
    Q_INVOKABLE QString GetRingtonePath();
    Q_INVOKABLE bool checkStartupLink();
    Q_INVOKABLE const QString getContactImageString(const QString& accountId, const QString& uid);
    Q_INVOKABLE void removeConversation(const QString& accountId,
                                        const QString& uid,
                                        bool banContact = false);
    Q_INVOKABLE void clearConversationHistory(const QString& accountId, const QString& uid);
    Q_INVOKABLE void setConversationFilter(const QString& filter);
    Q_INVOKABLE int getTotalUnreadMessages();
    Q_INVOKABLE int getTotalPendingRequest();
    Q_INVOKABLE const QString getBestName(const QString& accountId, const QString& uid);
    Q_INVOKABLE QString getBestId(const QString& accountId);
    Q_INVOKABLE const QString getBestId(const QString& accountId, const QString& uid);
    Q_INVOKABLE const QString getCurrAccId();
    Q_INVOKABLE const QString getCurrConvId();
    Q_INVOKABLE void makePermanentCurrentConv();
    Q_INVOKABLE const QStringList getCurrAccList();
    Q_INVOKABLE int getAccountListSize();
    Q_INVOKABLE void setCurrentCall(const QString& accountId, const QString& convUid);
    Q_INVOKABLE void startPreviewing(bool force);
    Q_INVOKABLE void stopPreviewing();
    Q_INVOKABLE bool hasVideoCall();
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
    Q_INVOKABLE QString getCroppedImageBase64FromFile(QString fileName, int size);
    Q_INVOKABLE bool checkShowPluginsButton();
    Q_INVOKABLE QString fileName(const QString& path);
    Q_INVOKABLE QString getExt(const QString& path);
    Q_INVOKABLE bool isImage(const QString& fileExt);
    Q_INVOKABLE QString humanFileSize(qint64 fileSize);

private:
    QClipboard* clipboard_;
};
Q_DECLARE_METATYPE(UtilsAdapter*)
