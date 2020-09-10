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

#include "utilsadapter.h"

#include "lrcinstance.h"
#include "utils.h"
#include "version.h"

#include <QApplication>
#include <QClipboard>
#include <QFileInfo>

UtilsAdapter::UtilsAdapter(QObject* parent)
    : QObject(parent)
    , clipboard_(QApplication::clipboard())
{}

const QString
UtilsAdapter::getChangeLog()
{
    return Utils::getChangeLog();
}

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
UtilsAdapter::setText(QString text)
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
UtilsAdapter::getContactImageString(const QString& accountId, const QString& uid)
{
    return Utils::getContactImageString(accountId, uid);
}

const QString
UtilsAdapter::getBestName(const QString& accountId, const QString& uid)
{
    auto* convModel = LRCInstance::getAccountInfo(accountId).conversationModel.get();
    return Utils::bestNameForConversation(convModel->getConversationForUID(uid), *convModel);
}

QString
UtilsAdapter::getBestId(const QString& accountId)
{
    if (accountId.isEmpty())
        return {};
    auto& accountInfo = LRCInstance::getAccountInfo(accountId);
    return Utils::bestIdForAccount(accountInfo);
}

const QString
UtilsAdapter::getBestId(const QString& accountId, const QString& uid)
{
    auto* convModel = LRCInstance::getAccountInfo(accountId).conversationModel.get();
    return Utils::bestIdForConversation(convModel->getConversationForUID(uid), *convModel);
}

int
UtilsAdapter::getTotalUnreadMessages()
{
    int totalUnreadMessages {0};
    if (LRCInstance::getCurrentAccountInfo().profileInfo.type != lrc::api::profile::Type::SIP) {
        auto* convModel = LRCInstance::getCurrentConversationModel();
        auto ringConversations = convModel->getFilteredConversations(lrc::api::profile::Type::RING);
        std::for_each(ringConversations.begin(),
                      ringConversations.end(),
                      [&totalUnreadMessages](const auto& conversation) {
                          totalUnreadMessages += conversation.unreadMessages;
                      });
    }
    return totalUnreadMessages;
}

int
UtilsAdapter::getTotalPendingRequest()
{
    auto& accountInfo = LRCInstance::getCurrentAccountInfo();
    return accountInfo.contactModel->pendingRequestCount();
}

void
UtilsAdapter::setConversationFilter(const QString& filter)
{
    LRCInstance::getCurrentConversationModel()->setFilter(filter);
}

void
UtilsAdapter::clearConversationHistory(const QString& accountId, const QString& uid)
{
    LRCInstance::getAccountInfo(accountId).conversationModel->clearHistory(uid);
}

void
UtilsAdapter::removeConversation(const QString& accountId, const QString& uid, bool banContact)
{
    LRCInstance::getAccountInfo(accountId).conversationModel->removeConversation(uid, banContact);
}

const QString
UtilsAdapter::getCurrAccId()
{
    return LRCInstance::getCurrAccId();
}

const QString
UtilsAdapter::getCurrConvId()
{
    return LRCInstance::getCurrentConvUid();
}

void
UtilsAdapter::makePermanentCurrentConv()
{
    LRCInstance::getCurrentConversationModel()->makePermanent(LRCInstance::getCurrentConvUid());
}

const QStringList
UtilsAdapter::getCurrAccList()
{
    return LRCInstance::accountModel().getAccountList();
}

int
UtilsAdapter::getAccountListSize()
{
    return getCurrAccList().size();
}

void
UtilsAdapter::setCurrentCall(const QString& accountId, const QString& convUid)
{
    auto& accInfo = LRCInstance::getAccountInfo(accountId);
    const auto convInfo = accInfo.conversationModel->getConversationForUID(convUid);
    accInfo.callModel->setCurrentCall(convInfo.callId);
}

void
UtilsAdapter::startPreviewing(bool force)
{
    LRCInstance::renderer()->startPreviewing(force);
}

void
UtilsAdapter::stopPreviewing()
{
    if (!LRCInstance::hasVideoCall()) {
        LRCInstance::renderer()->stopPreviewing();
    }
}

bool
UtilsAdapter::hasVideoCall()
{
    return LRCInstance::hasVideoCall();
}

const QString
UtilsAdapter::getCallId(const QString& accountId, const QString& convUid)
{
    auto& accInfo = LRCInstance::getAccountInfo(accountId);
    const auto convInfo = accInfo.conversationModel->getConversationForUID(convUid);

    if (convInfo.uid.isEmpty()) {
        return "";
    }

    auto call = LRCInstance::getCallInfoForConversation(convInfo, false);
    if (!call) {
        return "";
    }

    return call->id;
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
#ifdef Q_OS_WIN
    return path.replace("file:///", "").replace("\n", "").replace("\r", "");
#else
    return path.replace("file:///", "/").replace("\n", "").replace("\r", "");
#endif
}

QString
UtilsAdapter::getCroppedImageBase64FromFile(QString fileName, int size)
{
    auto image = Utils::cropImage(QImage(fileName));
    auto croppedImage = image.scaled(size,
                                     size,
                                     Qt::KeepAspectRatioByExpanding,
                                     Qt::SmoothTransformation);
    return QString::fromLatin1(Utils::QImageToByteArray(croppedImage).toBase64().data());
}

bool
UtilsAdapter::checkShowPluginsButton()
{
    return LRCInstance::pluginModel().getPluginsEnabled()
           && (LRCInstance::pluginModel().listLoadedPlugins().size() > 0);
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
