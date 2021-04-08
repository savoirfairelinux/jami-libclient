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
    connect(lrcInstance_, &LRCInstance::currentAccountChanged, [this]() {
        currentConvUid_.clear();
        connectConversationModel();
    });
    connectConversationModel();
}

void
MessagesAdapter::setupChatView(const QString& convUid)
{
    auto* convModel = lrcInstance_->getCurrentConversationModel();
    if (convModel == nullptr) {
        return;
    }

    if (currentConvUid_ == convUid)
        return;

    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid);
    if (convInfo.uid.isEmpty() || convInfo.participants.isEmpty()) {
        return;
    }

    QString contactURI = convInfo.participants.at(0);

    auto selectedAccountId = lrcInstance_->getCurrAccId();
    auto& accountInfo = lrcInstance_->accountModel().getAccountInfo(selectedAccountId);

    lrc::api::contact::Info contactInfo;
    try {
        contactInfo = accountInfo.contactModel->getContact(contactURI);
    } catch (...) {
    }

    bool shouldShowSendContactRequestBtn = (contactInfo.profileInfo.type
                                                == lrc::api::profile::Type::PENDING
                                            || contactInfo.profileInfo.type
                                                   == lrc::api::profile::Type::TEMPORARY);

    QMetaObject::invokeMethod(qmlObj_,
                              "setSendContactRequestButtonVisible",
                              Q_ARG(QVariant, shouldShowSendContactRequestBtn));

    setMessagesVisibility(false);

    /*
     * Type Indicator (contact).
     */
    contactIsComposing(convInfo.uid, "", false);
    connect(lrcInstance_->getCurrentConversationModel(),
            &ConversationModel::composingStatusChanged,
            [this](const QString& convUid, const QString& contactUri, bool isComposing) {
                if (!settingsManager_->getValue(Settings::Key::EnableTypingIndicator).toBool()) {
                    return;
                }
                contactIsComposing(convUid, contactUri, isComposing);
            });

    /*
     * Draft and message content set up.
     */
    Utils::oneShotConnect(qmlObj_,
                          SIGNAL(sendMessageContentSaved(const QString&)),
                          this,
                          SLOT(slotSendMessageContentSaved(const QString&)));

    requestSendMessageContent();

    currentConvUid_ = convUid;

    QString s = QString::fromLatin1("reset_message_bar_input(`%1`);")
                    .arg(accountInfo.contactModel->bestNameForContact(contactURI));
    QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
}

void
MessagesAdapter::connectConversationModel()
{
    auto currentConversationModel = lrcInstance_->getCurrentConversationModel();

    QObject::disconnect(newInteractionConnection_);
    QObject::disconnect(interactionRemovedConnection_);
    QObject::disconnect(interactionStatusUpdatedConnection_);

    newInteractionConnection_
        = QObject::connect(currentConversationModel,
                           &lrc::api::ConversationModel::newInteraction,
                           [this](const QString& convUid,
                                  uint64_t interactionId,
                                  const lrc::api::interaction::Info& interaction) {
                               auto accountId = lrcInstance_->getCurrAccId();
                               newInteraction(accountId, convUid, interactionId, interaction);
                           });

    interactionStatusUpdatedConnection_ = QObject::connect(
        currentConversationModel,
        &lrc::api::ConversationModel::interactionStatusUpdated,
        [this](const QString& convUid,
               uint64_t interactionId,
               const lrc::api::interaction::Info& interaction) {
            auto currentConversationModel = lrcInstance_->getCurrentConversationModel();
            currentConversationModel->clearUnreadInteractions(convUid);
            updateInteraction(*currentConversationModel, interactionId, interaction);
        });

    interactionRemovedConnection_
        = QObject::connect(currentConversationModel,
                           &lrc::api::ConversationModel::interactionRemoved,
                           [this](const QString& convUid, uint64_t interactionId) {
                               Q_UNUSED(convUid);
                               removeInteraction(interactionId);
                           });

    currentConversationModel->setFilter("");
}

void
MessagesAdapter::sendContactRequest()
{
    const auto convUid = lrcInstance_->getCurrentConvUid();
    if (!convUid.isEmpty()) {
        lrcInstance_->getCurrentConversationModel()->makePermanent(convUid);
    }
}

void
MessagesAdapter::updateConversationForAddedContact()
{
    auto* convModel = lrcInstance_->getCurrentConversationModel();
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(
        lrcInstance_->getCurrentConvUid());

    clear();
    setConversationProfileData(convInfo);
    printHistory(*convModel, convInfo.interactions);
}

void
MessagesAdapter::slotSendMessageContentSaved(const QString& content)
{
    if (!LastConvUid_.isEmpty()) {
        lrcInstance_->setContentDraft(LastConvUid_, lrcInstance_->getCurrAccId(), content);
    }
    LastConvUid_ = lrcInstance_->getCurrentConvUid();

    Utils::oneShotConnect(qmlObj_, SIGNAL(messagesCleared()), this, SLOT(slotMessagesCleared()));

    setInvitation(false);
    clear();
    auto restoredContent = lrcInstance_->getContentDraft(lrcInstance_->getCurrentConvUid(),
                                                         lrcInstance_->getCurrAccId());
    setSendMessageContent(restoredContent);
    Q_EMIT needToUpdateSmartList();
}

void
MessagesAdapter::slotUpdateDraft(const QString& content)
{
    if (!LastConvUid_.isEmpty()) {
        lrcInstance_->setContentDraft(LastConvUid_, lrcInstance_->getCurrAccId(), content);
    }
    Q_EMIT needToUpdateSmartList();
}

void
MessagesAdapter::slotMessagesCleared()
{
    auto* convModel = lrcInstance_->getCurrentConversationModel();
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(
        lrcInstance_->getCurrentConvUid());

    printHistory(*convModel, convInfo.interactions);

    Utils::oneShotConnect(qmlObj_, SIGNAL(messagesLoaded()), this, SLOT(slotMessagesLoaded()));

    setConversationProfileData(convInfo);
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
        const auto convUid = lrcInstance_->getCurrentConvUid();
        lrcInstance_->getCurrentConversationModel()->sendMessage(convUid, message);
    } catch (...) {
        qDebug() << "Exception during sendMessage:" << message;
    }
}

void
MessagesAdapter::sendImage(const QString& message)
{
    if (message.startsWith("data:image/png;base64,")) {
        /*
         * Img tag contains base64 data, trim "data:image/png;base64," from data.
         */
        QByteArray data = QByteArray::fromStdString(message.toStdString().substr(22));
        auto img_name_hash = QString::fromStdString(
            QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex().toStdString());
        QString fileName = "\\img_" + img_name_hash + ".png";

        QPixmap image_to_save;
        if (!image_to_save.loadFromData(QByteArray::fromBase64(data))) {
            qDebug().noquote() << "Errors during loadFromData"
                               << "\n";
        }

        QString path = QString(Utils::WinGetEnv("TEMP")) + fileName;
        if (!image_to_save.save(path, "PNG")) {
            qDebug().noquote() << "Errors during QPixmap save"
                               << "\n";
        }

        try {
            auto convUid = lrcInstance_->getCurrentConvUid();
            lrcInstance_->getCurrentConversationModel()->sendFile(convUid, path, fileName);
        } catch (...) {
            qDebug().noquote() << "Exception during sendFile - base64 img"
                               << "\n";
        }

    } else {
        /*
         * Img tag contains file paths.
         */

        // TODO: put all QRegExp strings together
        QString msg(message);
#ifdef Q_OS_WIN
        msg = msg.replace(QRegExp("^file:\\/{2,3}"), "");
#else
        msg = msg.replace(QRegExp("^file:\\/{2,3}"), "/");
#endif
        QFileInfo fi(msg);
        QString fileName = fi.fileName();

        try {
            auto convUid = lrcInstance_->getCurrentConvUid();
            lrcInstance_->getCurrentConversationModel()->sendFile(convUid, msg, fileName);
        } catch (...) {
            qDebug().noquote() << "Exception during sendFile - image from path"
                               << "\n";
        }
    }
}

void
MessagesAdapter::sendFile(const QString& message)
{
    QFileInfo fi(message);
    QString fileName = fi.fileName();
    try {
        auto convUid = lrcInstance_->getCurrentConvUid();
        lrcInstance_->getCurrentConversationModel()->sendFile(convUid, message, fileName);
    } catch (...) {
        qDebug() << "Exception during sendFile";
    }
}

void
MessagesAdapter::retryInteraction(const QString& arg)
{
    bool ok;
    uint64_t interactionUid = arg.toULongLong(&ok);
    if (ok) {
        lrcInstance_->getCurrentConversationModel()
            ->retryInteraction(lrcInstance_->getCurrentConvUid(), interactionUid);
    } else {
        qDebug() << "retryInteraction - invalid arg" << arg;
    }
}

void
MessagesAdapter::setNewMessagesContent(const QString& path)
{
    if (path.length() == 0)
        return;
    QByteArray imageFormat = QImageReader::imageFormat(path);

    if (!imageFormat.isEmpty()) {
        setMessagesImageContent(path);
    } else {
        setMessagesFileContent(path);
    }
}

void
MessagesAdapter::deleteInteraction(const QString& arg)
{
    bool ok;
    uint64_t interactionUid = arg.toULongLong(&ok);
    if (ok) {
        lrcInstance_->getCurrentConversationModel()
            ->clearInteractionFromConversation(lrcInstance_->getCurrentConvUid(), interactionUid);
    } else {
        qDebug() << "DeleteInteraction - invalid arg" << arg;
    }
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
MessagesAdapter::acceptFile(const QString& arg)
{
    try {
        auto interactionUid = arg.toLongLong();
        auto convUid = lrcInstance_->getCurrentConvUid();
        lrcInstance_->getCurrentConversationModel()->acceptTransfer(convUid, interactionUid);
    } catch (...) {
        qDebug() << "JS bridging - exception during acceptFile: " << arg;
    }
}

void
MessagesAdapter::refuseFile(const QString& arg)
{
    try {
        auto interactionUid = arg.toLongLong();
        const auto convUid = lrcInstance_->getCurrentConvUid();
        lrcInstance_->getCurrentConversationModel()->cancelTransfer(convUid, interactionUid);
    } catch (...) {
        qDebug() << "JS bridging - exception during refuseFile:" << arg;
    }
}

void
MessagesAdapter::pasteKeyDetected()
{
    const QMimeData* mimeData = QApplication::clipboard()->mimeData();

    if (mimeData->hasImage()) {
        /*
         * Save temp data into base64 format.
         */
        QPixmap pixmap = qvariant_cast<QPixmap>(mimeData->imageData());
        QByteArray ba;
        QBuffer bu(&ba);
        bu.open(QIODevice::WriteOnly);
        pixmap.save(&bu, "PNG");
        auto str = Utils::byteArrayToBase64String(ba);

        setMessagesImageContent(str, true);
    } else if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        /*
         * Extract the local paths of the files.
         */
        for (int i = 0; i < urlList.size(); ++i) {
            /*
             * Trim file:// or file:/// from url.
             */
            QString filePath = urlList.at(i).toString().remove(QRegExp("^file:\\/{2,3}"));
            QByteArray imageFormat = QImageReader::imageFormat(filePath);

            /*
             * Check if file is qt supported image file type.
             */
            if (!imageFormat.isEmpty()) {
                setMessagesImageContent(filePath);
            } else {
                setMessagesFileContent(filePath);
            }
        }
    } else {
        // Treat as text content, make chatview.js handle in order to
        // avoid string escape problems
        QMetaObject::invokeMethod(qmlObj_,
                                  "webViewRunJavaScript",
                                  Q_ARG(QVariant, QStringLiteral("replaceText()")));
    }
}

void
MessagesAdapter::onComposing(bool isComposing)
{
    if (!settingsManager_->getValue(Settings::Key::EnableTypingIndicator).toBool()) {
        return;
    }
    lrcInstance_->getCurrentConversationModel()->setIsComposing(lrcInstance_->getCurrentConvUid(),
                                                                isComposing);
}

void
MessagesAdapter::setConversationProfileData(const lrc::api::conversation::Info& convInfo)
{
    auto accInfo = &lrcInstance_->getCurrentAccountInfo();

    if (convInfo.participants.isEmpty()) {
        return;
    }

    auto contactUri = convInfo.participants.front();
    if (contactUri.isEmpty()) {
        return;
    }
    try {
        auto& contact = accInfo->contactModel->getContact(contactUri);
        auto bestName = accInfo->contactModel->bestNameForContact(contactUri);
        setInvitation(contact.profileInfo.type == lrc::api::profile::Type::PENDING
                          || contact.profileInfo.type == lrc::api::profile::Type::TEMPORARY,
                      bestName,
                      contactUri);

        if (!contact.profileInfo.avatar.isEmpty()) {
            setSenderImage(contactUri, contact.profileInfo.avatar);
        } else {
            auto avatar = Utils::contactPhoto(lrcInstance_, convInfo.participants[0]);
            QByteArray ba;
            QBuffer bu(&ba);
            avatar.save(&bu, "PNG");
            setSenderImage(contactUri, QString::fromLocal8Bit(ba.toBase64()));
        }
    } catch (...) {
    }
}

void
MessagesAdapter::newInteraction(const QString& accountId,
                                const QString& convUid,
                                uint64_t interactionId,
                                const interaction::Info& interaction)
{
    Q_UNUSED(interactionId);
    try {
        if (convUid.isEmpty() || convUid != lrcInstance_->getCurrentConvUid()) {
            return;
        }
        auto& accountInfo = lrcInstance_->getAccountInfo(accountId);
        auto& convModel = accountInfo.conversationModel;
        convModel->clearUnreadInteractions(convUid);
        printNewInteraction(*convModel, interactionId, interaction);
    } catch (...) {
    }
}

void
MessagesAdapter::updateDraft()
{
    currentConvUid_.clear();

    Utils::oneShotConnect(qmlObj_,
                          SIGNAL(sendMessageContentSaved(const QString&)),
                          this,
                          SLOT(slotUpdateDraft(const QString&)));

    requestSendMessageContent();
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
MessagesAdapter::requestSendMessageContent()
{
    QString s = QString::fromLatin1("requestSendMessageContent();");
    QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
}

void
MessagesAdapter::setInvitation(bool show, const QString& contactUri, const QString& contactId)
{
    QString s
        = show
              ? QString::fromLatin1("showInvitation(\"%1\", \"%2\")").arg(contactUri).arg(contactId)
              : QString::fromLatin1("showInvitation()");

    QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
}

void
MessagesAdapter::clear()
{
    QString s = QString::fromLatin1("clearMessages();");
    QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
}

void
MessagesAdapter::setDisplayLinks()
{
    QString s = QString::fromLatin1("setDisplayLinks(%1);")
                    .arg(settingsManager_->getValue(Settings::Key::DisplayImagesChatview).toBool());
    QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
}

void
MessagesAdapter::printHistory(lrc::api::ConversationModel& conversationModel,
                              const std::map<uint64_t, lrc::api::interaction::Info> interactions)
{
    auto interactionsStr = interactionsToJsonArrayObject(conversationModel, interactions).toUtf8();
    QString s = QString::fromLatin1("printHistory(%1);").arg(interactionsStr.constData());
    QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
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
                                     uint64_t msgId,
                                     const lrc::api::interaction::Info& interaction)
{
    auto interactionObject
        = interactionToJsonInteractionObject(conversationModel, msgId, interaction).toUtf8();
    if (interactionObject.isEmpty()) {
        return;
    }
    QString s = QString::fromLatin1("addMessage(%1);").arg(interactionObject.constData());
    QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
}

void
MessagesAdapter::updateInteraction(lrc::api::ConversationModel& conversationModel,
                                   uint64_t msgId,
                                   const lrc::api::interaction::Info& interaction)
{
    auto interactionObject
        = interactionToJsonInteractionObject(conversationModel, msgId, interaction).toUtf8();
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
    /*
     * If file name is too large, trim it.
     */
    if (fileName.length() > 15) {
        fileName = fileName.remove(12, fileName.length() - 12) + "...";
    }
    QString param = QString("addFile_path('%1','%2','%3')")
                        .arg(path, fileName, Utils::humanFileSize(fileSize));

    QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, param));
}

void
MessagesAdapter::removeInteraction(uint64_t interactionId)
{
    QString s = QString::fromLatin1("removeInteraction(%1);").arg(QString::number(interactionId));
    QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
}

void
MessagesAdapter::setSendMessageContent(const QString& content)
{
    QMetaObject::invokeMethod(qmlObj_, "setSendMessageContent", Q_ARG(QVariant, content));
}

void
MessagesAdapter::contactIsComposing(const QString& uid, const QString& contactUri, bool isComposing)
{
    if (lrcInstance_->getCurrentConvUid() == uid) {
        QString s
            = QString::fromLatin1("showTypingIndicator(`%1`, %2);").arg(contactUri).arg(isComposing);
        QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
    }
}

void
MessagesAdapter::acceptInvitation(const QString& convUid)
{
    const auto currentConvUid = convUid.isEmpty() ? lrcInstance_->getCurrentConvUid() : convUid;
    lrcInstance_->getCurrentConversationModel()->makePermanent(currentConvUid);
    if (convUid == currentConvUid_)
        currentConvUid_.clear();
    Q_EMIT invitationAccepted();
}

void
MessagesAdapter::refuseInvitation(const QString& convUid)
{
    const auto currentConvUid = convUid.isEmpty() ? lrcInstance_->getCurrentConvUid() : convUid;
    lrcInstance_->getCurrentConversationModel()->removeConversation(currentConvUid, false);
    setInvitation(false);
    if (convUid == currentConvUid_)
        currentConvUid_.clear();
    Q_EMIT navigateToWelcomePageRequested();
}

void
MessagesAdapter::blockConversation(const QString& convUid)
{
    const auto currentConvUid = convUid.isEmpty() ? lrcInstance_->getCurrentConvUid() : convUid;
    lrcInstance_->getCurrentConversationModel()->removeConversation(currentConvUid, true);
    setInvitation(false);
    if (convUid == currentConvUid_)
        currentConvUid_.clear();
    Q_EMIT contactBanned();
    Q_EMIT navigateToWelcomePageRequested();
}

void
MessagesAdapter::clearConversationHistory(const QString& accountId, const QString& convUid)
{
    lrcInstance_->getAccountInfo(accountId).conversationModel->clearHistory(convUid);
    if (convUid == currentConvUid_)
        currentConvUid_.clear();
}

void
MessagesAdapter::removeConversation(const QString& accountId,
                                    const QString& convUid,
                                    bool banContact)
{
    QStringList list = lrcInstance_->accountModel().getDefaultModerators(accountId);
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid, accountId);
    const auto contactURI = convInfo.participants.front();

    if (!contactURI.isEmpty() && list.contains(contactURI)) {
        lrcInstance_->accountModel().setDefaultModerator(accountId, contactURI, false);
    }

    lrcInstance_->getAccountInfo(accountId).conversationModel->removeConversation(convUid,
                                                                                  banContact);
    if (convUid == currentConvUid_)
        currentConvUid_.clear();
}
