/*!
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
#include "webchathelpers.h"

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

MessagesAdapter::MessagesAdapter(AppSettingsManager* settingsManager,
                                 LRCInstance* instance,
                                 QObject* parent)
    : QmlAdapterBase(instance, parent)
    , settingsManager_(settingsManager)
{}

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
    Utils::oneShotConnect(qmlObj_, SIGNAL(messagesCleared()), this, SLOT(slotMessagesCleared()));
    setMessagesVisibility(false);
    clearChatView();
    setIsSwarm(convInfo["isSwarm"].toBool());

    Q_EMIT newMessageBarPlaceholderText(convInfo["title"].toString());
}

void
MessagesAdapter::onNewInteraction(const QString& convUid,
                                  const QString& interactionId,
                                  const lrc::api::interaction::Info& interaction)
{
    auto accountId = lrcInstance_->get_currentAccountId();
    newInteraction(accountId, convUid, interactionId, interaction);
}

void
MessagesAdapter::onInteractionStatusUpdated(const QString& convUid,
                                            const QString& interactionId,
                                            const lrc::api::interaction::Info& interaction)
{
    auto currentConversationModel = lrcInstance_->getCurrentConversationModel();
    updateInteraction(*currentConversationModel, interactionId, interaction);
}

void
MessagesAdapter::onInteractionRemoved(const QString& convUid, const QString& interactionId)
{
    Q_UNUSED(convUid);
    removeInteraction(interactionId);
}

void
MessagesAdapter::onNewMessagesAvailable(const QString& accountId, const QString& conversationId)
{
    auto* convModel = lrcInstance_->accountModel().getAccountInfo(accountId).conversationModel.get();
    auto optConv = convModel->getConversationForUid(conversationId);
    if (!optConv)
        return;
    updateHistory(*convModel, optConv->get().interactions, optConv->get().allMessagesLoaded);
    Utils::oneShotConnect(qmlObj_, SIGNAL(messagesLoaded()), this, SLOT(slotMessagesLoaded()));
}

void
MessagesAdapter::updateConversation(const QString& conversationId)
{
    if (conversationId != lrcInstance_->get_selectedConvUid())
        return;
    auto* convModel = lrcInstance_->getCurrentConversationModel();
    if (auto optConv = convModel->getConversationForUid(conversationId))
        setConversationProfileData(optConv->get());
}

void
MessagesAdapter::onComposingStatusChanged(const QString& convId,
                                          const QString& contactUri,
                                          bool isComposing)
{
    if (convId != lrcInstance_->get_selectedConvUid())
        return;
    if (!settingsManager_->getValue(Settings::Key::EnableTypingIndicator).toBool()) {
        return;
    }
    contactIsComposing(contactUri, isComposing);
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
                     &ConversationModel::interactionStatusUpdated,
                     this,
                     &MessagesAdapter::onInteractionStatusUpdated,
                     Qt::UniqueConnection);

    QObject::connect(currentConversationModel,
                     &ConversationModel::interactionRemoved,
                     this,
                     &MessagesAdapter::onInteractionRemoved,
                     Qt::UniqueConnection);

    QObject::connect(currentConversationModel,
                     &ConversationModel::newMessagesAvailable,
                     this,
                     &MessagesAdapter::onNewMessagesAvailable,
                     Qt::UniqueConnection);

    QObject::connect(currentConversationModel,
                     &ConversationModel::conversationReady,
                     this,
                     &MessagesAdapter::updateConversation,
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
MessagesAdapter::slotMessagesCleared()
{
    auto* convModel = lrcInstance_->getCurrentConversationModel();

    auto optConv = convModel->getConversationForUid(lrcInstance_->get_selectedConvUid());
    if (!optConv)
        return;
    if (optConv->get().isSwarm() && !optConv->get().allMessagesLoaded) {
        convModel->loadConversationMessages(optConv->get().uid, 20);
    } else {
        updateHistory(*convModel, optConv->get().interactions, optConv->get().allMessagesLoaded);
        Utils::oneShotConnect(qmlObj_, SIGNAL(messagesLoaded()), this, SLOT(slotMessagesLoaded()));
    }
    setConversationProfileData(optConv->get());
}

void
MessagesAdapter::slotMessagesLoaded()
{
    setMessagesVisibility(true);
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
MessagesAdapter::openFile(const QString& arg)
{
    QUrl fileUrl("file:///" + arg);
    if (!QDesktopServices::openUrl(fileUrl)) {
        qDebug() << "Couldn't open file: " << fileUrl;
    }
}

void
MessagesAdapter::openUrl(const QString& url)
{
    if (!QDesktopServices::openUrl(url)) {
        qDebug() << "Couldn't open url: " << url;
    }
}

void
MessagesAdapter::acceptFile(const QString& interactionId)
{
    auto convUid = lrcInstance_->get_selectedConvUid();
    lrcInstance_->getCurrentConversationModel()->acceptTransfer(convUid, interactionId);
}

void
MessagesAdapter::refuseFile(const QString& interactionId)
{
    const auto convUid = lrcInstance_->get_selectedConvUid();
    lrcInstance_->getCurrentConversationModel()->cancelTransfer(convUid, interactionId);
}

void
MessagesAdapter::pasteKeyDetected()
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
MessagesAdapter::setConversationProfileData(const conversation::Info& convInfo)
{
    // make the all the participant avatars available within the web view
    for (const auto& participant : convInfo.participants) {
        QByteArray ba;
        QBuffer bu(&ba);
        Utils::conversationAvatar(lrcInstance_, convInfo.uid).save(&bu, "PNG");
        setSenderImage(participant, QString::fromLocal8Bit(ba.toBase64()));
    }
}

void
MessagesAdapter::newInteraction(const QString& accountId,
                                const QString& convUid,
                                const QString& interactionId,
                                const interaction::Info& interaction)
{
    Q_UNUSED(interactionId);
    try {
        if (convUid.isEmpty() || convUid != lrcInstance_->get_selectedConvUid()) {
            return;
        }
        auto& accountInfo = lrcInstance_->getAccountInfo(accountId);
        auto& convModel = accountInfo.conversationModel;
        convModel->clearUnreadInteractions(convUid);
        printNewInteraction(*convModel, interactionId, interaction);
        Q_EMIT newInteraction(static_cast<int>(interaction.type));
    } catch (...) {
    }
}

/*
 * JS invoke.
 */
void
MessagesAdapter::setMessagesVisibility(bool visible)
{
    QString s = QString::fromLatin1(visible ? "showMessagesDiv();" : "hideMessagesDiv();");
    QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
}

void
MessagesAdapter::setIsSwarm(bool isSwarm)
{
    QString s = QString::fromLatin1("set_is_swarm(%1)").arg(isSwarm);
    QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
}

void
MessagesAdapter::clearChatView()
{
    QString s = QString::fromLatin1("clearMessages();");
    QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
}

void
MessagesAdapter::setDisplayLinks()
{
    QString s
        = QString::fromLatin1("setDisplayLinks(%1);")
              .arg(settingsManager_->getValue(Settings::Key::DisplayHyperlinkPreviews).toBool());
    QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
}

void
MessagesAdapter::updateHistory(lrc::api::ConversationModel& conversationModel,
                               MessagesList interactions,
                               bool allLoaded)
{
    auto conversationId = lrcInstance_->get_selectedConvUid();
    auto interactionsStr
        = interactionsToJsonArrayObject(conversationModel, conversationId, interactions).toUtf8();
    QString s;
    QTextStream out(&s);
    out << "updateHistory(" << interactionsStr << ", " << (allLoaded? "true" : "false") << ");";
    QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
    conversationModel.clearUnreadInteractions(conversationId);
}

void
MessagesAdapter::setSenderImage(const QString& sender, const QString& senderImage)
{
    QJsonObject setSenderImageObject = QJsonObject();
    setSenderImageObject.insert("sender_contact_method", QJsonValue(sender));
    setSenderImageObject.insert("sender_image", QJsonValue(senderImage));

    auto setSenderImageObjectString = QString(
        QJsonDocument(setSenderImageObject).toJson(QJsonDocument::Compact));
    QString s = QString::fromLatin1("setSenderImage(%1);")
                    .arg(setSenderImageObjectString.toUtf8().constData());
    QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
}

void
MessagesAdapter::printNewInteraction(lrc::api::ConversationModel& conversationModel,
                                     const QString& msgId,
                                     const lrc::api::interaction::Info& interaction)
{
    auto interactionObject = interactionToJsonInteractionObject(conversationModel,
                                                                lrcInstance_->get_selectedConvUid(),
                                                                msgId,
                                                                interaction)
                                 .toUtf8();
    if (interactionObject.isEmpty()) {
        return;
    }
    QString s = QString::fromLatin1("addMessage(%1);").arg(interactionObject.constData());
    QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
}

void
MessagesAdapter::updateInteraction(lrc::api::ConversationModel& conversationModel,
                                   const QString& msgId,
                                   const lrc::api::interaction::Info& interaction)
{
    auto interactionObject = interactionToJsonInteractionObject(conversationModel,
                                                                lrcInstance_->get_selectedConvUid(),
                                                                msgId,
                                                                interaction)
                                 .toUtf8();
    if (interactionObject.isEmpty()) {
        return;
    }
    QString s = QString::fromLatin1("updateMessage(%1);").arg(interactionObject.constData());
    QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
}

void
MessagesAdapter::setMessagesImageContent(const QString& path, bool isBased64)
{
    if (isBased64) {
        QString param = QString("addImage_base64('%1')").arg(path);
        QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, param));
    } else {
        QString param = QString("addImage_path('file://%1')").arg(path);
        QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, param));
    }
}

void
MessagesAdapter::setMessagesFileContent(const QString& path)
{
    qint64 fileSize = QFileInfo(path).size();
    QString fileName = QFileInfo(path).fileName();

    QString param = QString("addFile_path('%1','%2','%3')")
                        .arg(path, fileName, Utils::humanFileSize(fileSize));

    QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, param));
}

void
MessagesAdapter::removeInteraction(const QString& interactionId)
{
    QString s = QString::fromLatin1("removeInteraction(%1);").arg(interactionId);
    QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
}

void
MessagesAdapter::setSendMessageContent(const QString& content)
{
    QMetaObject::invokeMethod(qmlObj_, "setSendMessageContent", Q_ARG(QVariant, content));
}

void
MessagesAdapter::contactIsComposing(const QString& contactUri, bool isComposing)
{
    auto* convModel = lrcInstance_->getCurrentConversationModel();
    auto convInfo = convModel->getConversationForUid(lrcInstance_->get_selectedConvUid());
    if (!convInfo)
        return;
    auto& conv = convInfo->get();
    bool showIsComposing = conv.participants.first() == contactUri;
    if (showIsComposing) {
        QString s
            = QString::fromLatin1("showTypingIndicator(`%1`, %2);").arg(contactUri).arg(isComposing);
        QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
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
MessagesAdapter::loadMessages(int n)
{
    auto* convModel = lrcInstance_->getCurrentConversationModel();
    auto convOpt = convModel->getConversationForUid(lrcInstance_->get_selectedConvUid());
    if (!convOpt)
        return;
    if (convOpt->get().isSwarm() && !convOpt->get().allMessagesLoaded)
        convModel->loadConversationMessages(convOpt->get().uid, n);
}
