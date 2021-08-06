/*!
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

#include "utilsadapter.h"

#include "lrcinstance.h"
#include "systemtray.h"
#include "utils.h"
#include "version.h"

#include "api/pluginmodel.h"

#include <QApplication>
#include <QClipboard>
#include <QFileInfo>

UtilsAdapter::UtilsAdapter(SystemTray* systemTray, LRCInstance* instance, QObject* parent)
    : QmlAdapterBase(instance, parent)
    , clipboard_(QApplication::clipboard())
    , systemTray_(systemTray)
{}

const QString
UtilsAdapter::getProjectCredits()
{
    return Utils::getProjectCredits();
}

const QString
UtilsAdapter::getVersionStr()
{
    return QString(VERSION_STRING);
}

void
UtilsAdapter::setClipboardText(QString text)
{
    clipboard_->setText(text, QClipboard::Clipboard);
}

const QString
UtilsAdapter::qStringFromFile(const QString& filename)
{
    return Utils::QByteArrayFromFile(filename);
}

const QString
UtilsAdapter::getStyleSheet(const QString& name, const QString& source)
{
    auto simplifiedCSS = source.simplified().replace("'", "\"");
    QString s = QString::fromLatin1("(function() {"
                                    "    var node = document.createElement('style');"
                                    "    node.id = '%1';"
                                    "    node.innerHTML = '%2';"
                                    "    document.head.appendChild(node);"
                                    "})()")
                    .arg(name)
                    .arg(simplifiedCSS);
    return s;
}

const QString
UtilsAdapter::getCachePath()
{
    QDir dataDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    dataDir.cdUp();
    return dataDir.absolutePath() + "/jami";
}
bool
UtilsAdapter::createStartupLink()
{
    return Utils::CreateStartupLink(L"Jami");
}

QString
UtilsAdapter::GetRingtonePath()
{
    return Utils::GetRingtonePath();
}

bool
UtilsAdapter::checkStartupLink()
{
    return Utils::CheckStartupLink(L"Jami");
}

const QString
UtilsAdapter::getBestName(const QString& accountId, const QString& uid)
{
    const auto& conv = lrcInstance_->getConversationFromConvUid(uid);
    if (!conv.participants.isEmpty())
        return lrcInstance_->getAccountInfo(accountId).contactModel->bestNameForContact(
            conv.participants[0]);
    return QString();
}

const QString
UtilsAdapter::getPeerUri(const QString& accountId, const QString& uid)
{
    try {
        auto* convModel = lrcInstance_->getAccountInfo(accountId).conversationModel.get();
        const auto& convInfo = convModel->getConversationForUid(uid).value();
        return convInfo.get().participants.front();
    } catch (const std::out_of_range& e) {
        qDebug() << e.what();
        return "";
    }
}

QString
UtilsAdapter::getBestId(const QString& accountId)
{
    if (accountId.isEmpty())
        return {};
    return lrcInstance_->accountModel().bestIdForAccount(accountId);
}

const QString
UtilsAdapter::getBestId(const QString& accountId, const QString& uid)
{
    const auto& conv = lrcInstance_->getConversationFromConvUid(uid);
    if (!conv.participants.isEmpty())
        return lrcInstance_->getAccountInfo(accountId).contactModel->bestIdForContact(
            conv.participants[0]);
    return QString();
}

void
UtilsAdapter::setConversationFilter(const QString& filter)
{
    lrcInstance_->getCurrentConversationModel()->setFilter(filter);
}

const QStringList
UtilsAdapter::getCurrAccList()
{
    return lrcInstance_->accountModel().getAccountList();
}

int
UtilsAdapter::getAccountListSize()
{
    return getCurrAccList().size();
}

bool
UtilsAdapter::hasCall(const QString& accountId)
{
    auto activeCalls = lrcInstance_->getActiveCalls();
    for (const auto& callId : activeCalls) {
        auto& accountInfo = lrcInstance_->accountModel().getAccountInfo(accountId);
        if (accountInfo.callModel->hasCall(callId)) {
            return true;
        }
    }
    return false;
}

const QString
UtilsAdapter::getCallConvForAccount(const QString& accountId)
{
    // TODO: Currently returning first call, establish priority according to state?
    for (const auto& callId : lrcInstance_->getActiveCalls()) {
        auto& accountInfo = lrcInstance_->accountModel().getAccountInfo(accountId);
        if (accountInfo.callModel->hasCall(callId)) {
            return lrcInstance_->getConversationFromCallId(callId, accountId).uid;
        }
    }
    return "";
}

const QString
UtilsAdapter::getCallId(const QString& accountId, const QString& convUid)
{
    auto const& convInfo = lrcInstance_->getConversationFromConvUid(convUid, accountId);
    if (convInfo.uid.isEmpty()) {
        return {};
    }

    if (auto* call = lrcInstance_->getCallInfoForConversation(convInfo, false)) {
        return call->id;
    }

    return {};
}

int
UtilsAdapter::getCallStatus(const QString& callId)
{
    const auto callStatus = lrcInstance_->getCallInfo(callId, lrcInstance_->get_currentAccountId());
    return static_cast<int>(callStatus->status);
}

const QString
UtilsAdapter::getCallStatusStr(int statusInt)
{
    const auto status = static_cast<lrc::api::call::Status>(statusInt);
    return lrc::api::call::to_string(status);
}

// returns true if name is valid registered name
bool
UtilsAdapter::validateRegNameForm(const QString& regName)
{
    QRegularExpression regExp(" ");

    if (regName.size() > 2 && !regName.contains(regExp)) {
        return true;

    } else {
        return false;
    }
}

QString
UtilsAdapter::getStringUTF8(QString string)
{
    return string.toUtf8();
}

QString
UtilsAdapter::getRecordQualityString(int value)
{
    return value ? QString::number(static_cast<float>(value) / 100, 'f', 1) + " Mbps" : "Default";
}

QString
UtilsAdapter::getCurrentPath()
{
    return QDir::currentPath();
}

QString
UtilsAdapter::stringSimplifier(QString input)
{
    return input.simplified();
}

QString
UtilsAdapter::toNativeSeparators(QString inputDir)
{
    return QDir::toNativeSeparators(inputDir);
}

QString
UtilsAdapter::toFileInfoName(QString inputFileName)
{
    QFileInfo fi(inputFileName);
    return fi.fileName();
}

QString
UtilsAdapter::toFileAbsolutepath(QString inputFileName)
{
    QFileInfo fi(inputFileName);
    return fi.absolutePath();
}

QString
UtilsAdapter::getAbsPath(QString path)
{
    // Note: this function is used on urls returned from qml-FileDialogs which
    // contain 'file:///' for reasons we don't understand.
    // TODO: this logic can be refactored into the JamiFileDialog component.
#ifdef Q_OS_WIN
    return path.replace(QRegExp("^file:\\/{2,3}"), "").replace("\n", "").replace("\r", "");
#else
    return path.replace(QRegExp("^file:\\/{2,3}"), "/").replace("\n", "").replace("\r", "");
#endif
}

QString
UtilsAdapter::fileName(const QString& path)
{
    QFileInfo fi(path);
    return fi.fileName();
}

QString
UtilsAdapter::getExt(const QString& path)
{
    QFileInfo fi(path);
    return fi.completeSuffix();
}

bool
UtilsAdapter::isImage(const QString& fileExt)
{
    return Utils::isImage(fileExt);
}

QString
UtilsAdapter::humanFileSize(qint64 fileSize)
{
    return Utils::humanFileSize(fileSize);
}

void
UtilsAdapter::setSystemTrayIconVisible(bool visible)
{
    systemTray_->setVisible(visible);
}
