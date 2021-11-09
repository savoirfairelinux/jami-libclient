/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>
 * Author: Anthony Léonard <anthony.leonard@savoirfairelinux.com>
 * Author: Olivier Soldano <olivier.soldano@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 * Author: Isa Nanic <isa.nanic@savoirfairelinux.com>
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
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

#include "messagesadapter.h"

#include "appsettingsmanager.h"
#include "qtutils.h"
#include "utils.h"

#include <api/datatransfermodel.h>

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QFileInfo>
#include <QImageReader>
#include <QList>
#include <QUrl>
#include <QMimeData>
#include <QBuffer>
#include <QtMath>

MessagesAdapter::MessagesAdapter(AppSettingsManager* settingsManager,
                                 PreviewEngine* previewEngine,
                                 LRCInstance* instance,
                                 QObject* parent)
    : QmlAdapterBase(instance, parent)
    , settingsManager_(settingsManager)
    , previewEngine_(previewEngine)
    , filteredMsgListModel_(new FilteredMsgListModel(this))
{
    connect(lrcInstance_, &LRCInstance::selectedConvUidChanged, [this]() {
        const QString& convId = lrcInstance_->get_selectedConvUid();
        const auto& conversation = lrcInstance_->getConversationFromConvUid(convId);
        filteredMsgListModel_->setSourceModel(conversation.interactions.get());
        set_messageListModel(QVariant::fromValue(filteredMsgListModel_));
        if (!conversation.typers.empty())
            set_currentConvComposingList(conversationTypersUrlToName(conversation.typers));
        else
            set_currentConvComposingList({});
    });

    connect(previewEngine_, &PreviewEngine::infoReady, this, &MessagesAdapter::onPreviewInfoReady);
    connect(previewEngine_,
            &PreviewEngine::linkifyReady,
            this,
            &MessagesAdapter::onMessageLinkified);
}

void
MessagesAdapter::safeInit()
{
    connect(lrcInstance_, &LRCInstance::currentAccountIdChanged, [this]() {
        connectConversationModel();
    });
    connectConversationModel();
}

void
MessagesAdapter::setupChatView(const QVariantMap& convInfo)
{
    auto* convModel = lrcInstance_->getCurrentConversationModel();
    auto convId = convInfo["convId"].toString();
    if (convInfo["isSwarm"].toBool()) {
        convModel->loadConversationMessages(convId, loadChunkSize_);
    }

    // TODO: current conv observe
    Q_EMIT newMessageBarPlaceholderText(convInfo["title"].toString());
}

void
MessagesAdapter::loadMoreMessages()
{
    auto accountId = lrcInstance_->get_currentAccountId();
    auto convId = lrcInstance_->get_selectedConvUid();
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convId, accountId);
    if (convInfo.isSwarm()) {
        auto* convModel = lrcInstance_->getCurrentConversationModel();
        convModel->loadConversationMessages(convId, loadChunkSize_);
    }
}

void
MessagesAdapter::connectConversationModel()
{
    auto currentConversationModel = lrcInstance_->getCurrentConversationModel();

    QObject::connect(currentConversationModel,
                     &ConversationModel::newInteraction,
                     this,
                     &MessagesAdapter::onNewInteraction,
                     Qt::UniqueConnection);

    QObject::connect(currentConversationModel,
                     &ConversationModel::conversationMessagesLoaded,
                     this,
                     &MessagesAdapter::onConversationMessagesLoaded,
                     Qt::UniqueConnection);

    QObject::connect(currentConversationModel,
                     &ConversationModel::composingStatusChanged,
                     this,
                     &MessagesAdapter::onComposingStatusChanged,
                     Qt::UniqueConnection);
}

void
MessagesAdapter::sendConversationRequest()
{
    lrcInstance_->makeConversationPermanent();
}

void
MessagesAdapter::sendMessage(const QString& message)
{
    try {
        const auto convUid = lrcInstance_->get_selectedConvUid();
        lrcInstance_->getCurrentConversationModel()->sendMessage(convUid, message);
    } catch (...) {
        qDebug() << "Exception during sendMessage:" << message;
    }
}

void
MessagesAdapter::sendFile(const QString& message)
{
    QFileInfo fi(message);
    QString fileName = fi.fileName();
    try {
        auto convUid = lrcInstance_->get_selectedConvUid();
        lrcInstance_->getCurrentConversationModel()->sendFile(convUid, message, fileName);
    } catch (...) {
        qDebug() << "Exception during sendFile";
    }
}

void
MessagesAdapter::retryInteraction(const QString& interactionId)
{
    lrcInstance_->getCurrentConversationModel()
        ->retryInteraction(lrcInstance_->get_selectedConvUid(), interactionId);
}

void
MessagesAdapter::copyToDownloads(const QString& interactionId, const QString& displayName)
{
    auto downloadDir = lrcInstance_->accountModel().downloadDirectory;
    if (auto accInfo = &lrcInstance_->getCurrentAccountInfo())
        accInfo->dataTransferModel->copyTo(lrcInstance_->get_currentAccountId(),
                                           lrcInstance_->get_selectedConvUid(),
                                           interactionId,
                                           downloadDir,
                                           displayName);
}

void
MessagesAdapter::deleteInteraction(const QString& interactionId)
{
    lrcInstance_->getCurrentConversationModel()
        ->clearInteractionFromConversation(lrcInstance_->get_selectedConvUid(), interactionId);
}

void
MessagesAdapter::openUrl(const QString& url)
{
    if (!QDesktopServices::openUrl(url)) {
        qDebug() << "Couldn't open url: " << url;
    }
}

void
MessagesAdapter::openDirectory(const QString& path)
{
    QString p = path;
    QFileInfo f(p);
    if (f.exists()) {
        if (!f.isDir())
            p = f.dir().absolutePath();
        QString url;
        if (!p.startsWith("file:/"))
            url = "file:///" + p;
        else
            url = p;
        openUrl(url);
    }
}

void
MessagesAdapter::acceptFile(const QString& interactionId)
{
    auto convUid = lrcInstance_->get_selectedConvUid();
    lrcInstance_->getCurrentConversationModel()->acceptTransfer(convUid, interactionId);
}

void
MessagesAdapter::cancelFile(const QString& interactionId)
{
    const auto convUid = lrcInstance_->get_selectedConvUid();
    lrcInstance_->getCurrentConversationModel()->cancelTransfer(convUid, interactionId);
}

void
MessagesAdapter::onPaste()
{
    const QMimeData* mimeData = QApplication::clipboard()->mimeData();

    if (mimeData->hasImage()) {
        // Save temp data into a temp file.
        QPixmap pixmap = qvariant_cast<QPixmap>(mimeData->imageData());

        auto img_name_hash
            = QCryptographicHash::hash(QString::number(pixmap.cacheKey()).toLocal8Bit(),
                                       QCryptographicHash::Sha1);
        QString fileName = "\\img_" + QString(img_name_hash.toHex()) + ".png";
        QString path = QString(Utils::WinGetEnv("TEMP")) + fileName;

        if (!pixmap.save(path, "PNG")) {
            qDebug().noquote() << "Errors during QPixmap save"
                               << "\n";
            return;
        }

        Q_EMIT newFilePasted(path);
    } else if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();

        // Extract the local paths of the files.
        for (int i = 0; i < urlList.size(); ++i) {
            // Trim file:// or file:/// from url.
            QString filePath = urlList.at(i).toString().remove(QRegExp("^file:\\/{2,3}"));
            Q_EMIT newFilePasted(filePath);
        }
    } else {
        // Treat as text content, make chatview.js handle in order to
        // avoid string escape problems
        Q_EMIT newTextPasted();
    }
}

QString
MessagesAdapter::getStatusString(int status)
{
    switch (static_cast<interaction::Status>(status)) {
    case interaction::Status::SENDING:
        return QObject::tr("Sending");
    case interaction::Status::FAILURE:
        return QObject::tr("Failure");
    case interaction::Status::SUCCESS:
        return QObject::tr("Sent");
    case interaction::Status::TRANSFER_CREATED:
        return QObject::tr("Connecting");
    case interaction::Status::TRANSFER_ACCEPTED:
        return QObject::tr("Accept");
    case interaction::Status::TRANSFER_CANCELED:
        return QObject::tr("Canceled");
    case interaction::Status::TRANSFER_ERROR:
    case interaction::Status::TRANSFER_UNJOINABLE_PEER:
        return QObject::tr("Unable to make contact");
    case interaction::Status::TRANSFER_ONGOING:
        return QObject::tr("Ongoing");
    case interaction::Status::TRANSFER_AWAITING_PEER:
        return QObject::tr("Waiting for contact");
    case interaction::Status::TRANSFER_AWAITING_HOST:
        return QObject::tr("Incoming transfer");
    case interaction::Status::TRANSFER_TIMEOUT_EXPIRED:
        return QObject::tr("Timed out waiting for contact");
    case interaction::Status::TRANSFER_FINISHED:
        return QObject::tr("Finished");
    default:
        return {};
    }
}

QVariantMap
MessagesAdapter::getTransferStats(const QString& msgId, int status)
{
    Q_UNUSED(status)
    auto convModel = lrcInstance_->getCurrentConversationModel();
    lrc::api::datatransfer::Info info = {};
    convModel->getTransferInfo(lrcInstance_->get_selectedConvUid(), msgId, info);
    return {{"totalSize", qint64(info.totalSize)}, {"progress", qint64(info.progress)}};
}

void
MessagesAdapter::userIsComposing(bool isComposing)
{
    if (!settingsManager_->getValue(Settings::Key::EnableTypingIndicator).toBool()
        || lrcInstance_->get_selectedConvUid().isEmpty()) {
        return;
    }
    lrcInstance_->getCurrentConversationModel()->setIsComposing(lrcInstance_->get_selectedConvUid(),
                                                                isComposing);
}

void
MessagesAdapter::onNewInteraction(const QString& convUid,
                                  const QString& interactionId,
                                  const interaction::Info& interaction)
{
    Q_UNUSED(interactionId);
    try {
        if (convUid.isEmpty() || convUid != lrcInstance_->get_selectedConvUid()) {
            return;
        }
        auto accountId = lrcInstance_->get_currentAccountId();
        auto& accountInfo = lrcInstance_->getAccountInfo(accountId);
        auto& convModel = accountInfo.conversationModel;
        convModel->clearUnreadInteractions(convUid);
        Q_EMIT newInteraction(static_cast<int>(interaction.type));
    } catch (...) {
    }
}

void
MessagesAdapter::acceptInvitation(const QString& convId)
{
    auto conversationId = convId.isEmpty() ? lrcInstance_->get_selectedConvUid() : convId;
    auto* convModel = lrcInstance_->getCurrentConversationModel();
    convModel->acceptConversationRequest(conversationId);
}

void
MessagesAdapter::refuseInvitation(const QString& convUid)
{
    const auto currentConvUid = convUid.isEmpty() ? lrcInstance_->get_selectedConvUid() : convUid;
    lrcInstance_->getCurrentConversationModel()->removeConversation(currentConvUid, false);
}

void
MessagesAdapter::blockConversation(const QString& convUid)
{
    const auto currentConvUid = convUid.isEmpty() ? lrcInstance_->get_selectedConvUid() : convUid;
    lrcInstance_->getCurrentConversationModel()->removeConversation(currentConvUid, true);
}

void
MessagesAdapter::unbanContact(int index)
{
    auto& accountInfo = lrcInstance_->getCurrentAccountInfo();
    auto bannedContactList = accountInfo.contactModel->getBannedContacts();
    auto it = bannedContactList.begin();
    std::advance(it, index);

    try {
        auto contactInfo = accountInfo.contactModel->getContact(*it);
        accountInfo.contactModel->addContact(contactInfo);
    } catch (const std::out_of_range& e) {
        qDebug() << e.what();
    }
}

void
MessagesAdapter::clearConversationHistory(const QString& accountId, const QString& convUid)
{
    lrcInstance_->getAccountInfo(accountId).conversationModel->clearHistory(convUid);
}

void
MessagesAdapter::removeConversation(const QString& convUid)
{
    auto& accInfo = lrcInstance_->getCurrentAccountInfo();
    accInfo.conversationModel->removeConversation(convUid);
}

void
MessagesAdapter::removeContact(const QString& convUid, bool banContact)
{
    auto& accInfo = lrcInstance_->getCurrentAccountInfo();

    // remove the uri from the default moderators list
    // TODO: seems like this should be done in libringclient
    QStringList list = lrcInstance_->accountModel().getDefaultModerators(accInfo.id);
    const auto contactUri = accInfo.conversationModel->peersForConversation(convUid).at(0);
    if (!contactUri.isEmpty() && list.contains(contactUri)) {
        lrcInstance_->accountModel().setDefaultModerator(accInfo.id, contactUri, false);
    }

    // actually remove the contact
    accInfo.contactModel->removeContact(contactUri, banContact);
}

void
MessagesAdapter::onPreviewInfoReady(QString messageId, QVariantMap info)
{
    const QString& convId = lrcInstance_->get_selectedConvUid();
    const QString& accId = lrcInstance_->get_currentAccountId();
    auto& conversation = lrcInstance_->getConversationFromConvUid(convId, accId);
    conversation.interactions->addHyperlinkInfo(messageId, info);
}

void
MessagesAdapter::onConversationMessagesLoaded(uint32_t, const QString& convId)
{
    if (convId != lrcInstance_->get_selectedConvUid())
        return;
    Q_EMIT moreMessagesLoaded();
}

void
MessagesAdapter::parseMessageUrls(const QString& messageId, const QString& msg, bool showPreview)
{
    previewEngine_->parseMessage(messageId, msg, showPreview);
}

void
MessagesAdapter::onMessageLinkified(const QString& messageId, const QString& linkified)
{
    const QString& convId = lrcInstance_->get_selectedConvUid();
    const QString& accId = lrcInstance_->get_currentAccountId();
    auto& conversation = lrcInstance_->getConversationFromConvUid(convId, accId);
    conversation.interactions->linkifyMessage(messageId, linkified);
}

void
MessagesAdapter::onComposingStatusChanged(const QString& convId,
                                          const QString& contactUri,
                                          bool isComposing)
{
    Q_UNUSED(contactUri)
    if (lrcInstance_->get_selectedConvUid() == convId) {
        const QString& accId = lrcInstance_->get_currentAccountId();
        auto& conversation = lrcInstance_->getConversationFromConvUid(convId, accId);
        set_currentConvComposingList(conversationTypersUrlToName(conversation.typers));
    }
}

QList<QString>
MessagesAdapter::conversationTypersUrlToName(const QSet<QString>& typersSet)
{
    QList<QString> nameList;
    for (const auto& id : typersSet) {
        auto name = lrcInstance_->getCurrentContactModel()->bestNameForContact(id);
        nameList.append(name);
    }

    return nameList;
}

bool
MessagesAdapter::isLocalImage(const QString& msg)
{
    QImageReader reader;
    reader.setDecideFormatFromContent(true);
    reader.setFileName(msg);
    return !reader.read().isNull();
}

QVariantMap
MessagesAdapter::getMediaInfo(const QString& msg)
{
    auto filePath = QFileInfo(msg).absoluteFilePath();
    static const QString html
        = "<body style='margin:0;padding:0;'>"
          "<%1 style='width:100%;height:%2;outline:none;background-color:#f1f3f4;"
          "object-fit:cover;' "
          "controls controlsList='nodownload' src='file://%3' type='%4'/></body>";
    if (isLocalImage(msg)) {
        return {{"isImage", true}};
    }
    QRegularExpression vPattern("[^\\s]+(.*?)\\.(avi|mov|webm|webp|rmvb)$",
                                QRegularExpression::CaseInsensitiveOption);
    QString type = vPattern.match(filePath).captured(2);
    if (!type.isEmpty()) {
        return {
            {"isVideo", true},
            {"html", html.arg("video", "100%", filePath, "video/" + type)},
        };
    } else {
        QRegularExpression aPattern("[^\\s]+(.*?)\\.(ogg|flac|wav|mpeg|mp3)$",
                                    QRegularExpression::CaseInsensitiveOption);
        type = aPattern.match(filePath).captured(2);
        if (!type.isEmpty()) {
            return {
                {"isVideo", false},
                {"html", html.arg("audio", "54px", filePath, "audio/" + type)},
            };
        }
    }
    return {};
}

bool
MessagesAdapter::isRemoteImage(const QString& msg)
{
    // TODO: test if all these open in the AnimatedImage component
    QRegularExpression pattern("[^\\s]+(.*?)\\.(jpg|jpeg|png|gif|apng|webp|avif|flif)$",
                               QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = pattern.match(msg);
    return match.hasMatch();
}

QString
MessagesAdapter::getFormattedTime(const quint64 timestamp)
{
    const auto now = QDateTime::currentDateTime();
    const auto seconds = now.toSecsSinceEpoch() - timestamp;
    auto interval = qFloor(seconds / (3600 * 24));
    if (interval > 5)
        return QLocale::system().toString(QDateTime::fromSecsSinceEpoch(timestamp),
                                          QLocale::ShortFormat);
    if (interval > 1)
        return QObject::tr("%1 days ago").arg(interval);
    if (interval == 1)
        return QObject::tr("one day ago");
    interval = qFloor(seconds / 3600);
    if (interval > 1)
        return QObject::tr("%1 hours ago").arg(interval);
    if (interval == 1)
        return QObject::tr("one hour ago");
    interval = qFloor(seconds / 60);
    if (interval > 1)
        return QObject::tr("%1 minutes ago").arg(interval);
    return QObject::tr("just now");
}
