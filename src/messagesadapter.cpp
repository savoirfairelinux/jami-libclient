/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>
 * Author: Anthony L�onard <anthony.leonard@savoirfairelinux.com>
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
#include "webchathelpers.h"

#include "utils.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QImageReader>
#include <QList>
#include <QUrl>

MessagesAdapter::MessagesAdapter(QObject *parent)
    : QmlAdapterBase(parent)
{}

MessagesAdapter::~MessagesAdapter() {}

void
MessagesAdapter::initQmlObject()
{
    connectConversationModel();
}

void
MessagesAdapter::setupChatView(const QString &uid)
{
    auto &convInfo = LRCInstance::getConversationFromConvUid(uid);
    if (convInfo.uid.isEmpty()) {
        return;
    }

    QString contactURI = convInfo.participants.at(0);

    bool isContact = false;
    auto selectedAccountId = LRCInstance::getCurrAccId();
    auto &accountInfo = LRCInstance::accountModel().getAccountInfo(selectedAccountId);

    lrc::api::profile::Type contactType;
    try {
        auto contactInfo = accountInfo.contactModel->getContact(contactURI);
        if (contactInfo.isTrusted) {
            isContact = true;
        }
        contactType = contactInfo.profileInfo.type;
    } catch (...) {
    }

    bool shouldShowSendContactRequestBtn = !isContact
                                           && contactType != lrc::api::profile::Type::SIP;

    QMetaObject::invokeMethod(qmlObj_,
                              "setSendContactRequestButtonVisible",
                              Q_ARG(QVariant, shouldShowSendContactRequestBtn));

    setMessagesVisibility(false);

    /*
     * Type Indicator (contact).
     */
    contactIsComposing(convInfo.uid, "", false);
    connect(LRCInstance::getCurrentConversationModel(),
            &ConversationModel::composingStatusChanged,
            [this](const QString &uid, const QString &contactUri, bool isComposing) {
                contactIsComposing(uid, contactUri, isComposing);
            });

    /*
     * Draft and message content set up.
     */
    Utils::oneShotConnect(qmlObj_,
                          SIGNAL(sendMessageContentSaved(const QString &)),
                          this,
                          SLOT(slotSendMessageContentSaved(const QString &)));

    requestSendMessageContent();
}

void
MessagesAdapter::connectConversationModel()
{
    auto currentConversationModel = LRCInstance::getCurrentAccountInfo().conversationModel.get();

    QObject::disconnect(newInteractionConnection_);
    QObject::disconnect(interactionRemovedConnection_);
    QObject::disconnect(interactionStatusUpdatedConnection_);

    newInteractionConnection_
        = QObject::connect(currentConversationModel,
                           &lrc::api::ConversationModel::newInteraction,
                           [this](const QString &convUid,
                                  uint64_t interactionId,
                                  const lrc::api::interaction::Info &interaction) {
                               auto accountId = LRCInstance::getCurrAccId();
                               newInteraction(accountId, convUid, interactionId, interaction);
                           });

    interactionStatusUpdatedConnection_ = QObject::connect(
        currentConversationModel,
        &lrc::api::ConversationModel::interactionStatusUpdated,
        [this](const QString &convUid,
               uint64_t interactionId,
               const lrc::api::interaction::Info &interaction) {
            if (convUid != LRCInstance::getCurrentConvUid()) {
                return;
            }
            auto &currentAccountInfo = LRCInstance::getCurrentAccountInfo();
            auto currentConversationModel = currentAccountInfo.conversationModel.get();
            currentConversationModel->clearUnreadInteractions(convUid);
            updateInteraction(*currentConversationModel, interactionId, interaction);
        });

    interactionRemovedConnection_
        = QObject::connect(currentConversationModel,
                           &lrc::api::ConversationModel::interactionRemoved,
                           [this](const QString &convUid, uint64_t interactionId) {
                               Q_UNUSED(convUid);
                               removeInteraction(interactionId);
                           });

    currentConversationModel->setFilter("");
}

void
MessagesAdapter::sendContactRequest()
{
    auto convInfo = LRCInstance::getCurrentConversation();
    if (!convInfo.uid.isEmpty()) {
        LRCInstance::getCurrentConversationModel()->makePermanent(convInfo.uid);
    }
}

void
MessagesAdapter::accountChangedSetUp(const QString &accoountId)
{
    Q_UNUSED(accoountId)

    connectConversationModel();
}

void
MessagesAdapter::updateConversationForAddedContact()
{
    auto conversation = LRCInstance::getCurrentConversation();
    auto convModel = LRCInstance::getCurrentConversationModel();

    clear();
    setConversationProfileData(conversation);
    printHistory(*convModel, conversation.interactions);
}

void
MessagesAdapter::slotSendMessageContentSaved(const QString &content)
{
    if (!LastConvUid_.isEmpty()) {
        LRCInstance::setContentDraft(LastConvUid_, LRCInstance::getCurrAccId(), content);
    }
    LastConvUid_ = LRCInstance::getCurrentConvUid();

    Utils::oneShotConnect(qmlObj_, SIGNAL(messagesCleared()), this, SLOT(slotMessagesCleared()));

    setInvitation(false);
    clear();
    auto restoredContent = LRCInstance::getContentDraft(LRCInstance::getCurrentConvUid(),
                                                        LRCInstance::getCurrAccId());
    setSendMessageContent(restoredContent);
    emit needToUpdateSmartList();
}

void
MessagesAdapter::slotUpdateDraft(const QString &content)
{
    if (!LastConvUid_.isEmpty()) {
        LRCInstance::setContentDraft(LastConvUid_, LRCInstance::getCurrAccId(), content);
    }
    emit needToUpdateSmartList();
}

void
MessagesAdapter::slotMessagesCleared()
{
    auto &convInfo = LRCInstance::getConversationFromConvUid(LRCInstance::getCurrentConvUid());
    auto convModel = LRCInstance::getCurrentConversationModel();

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
MessagesAdapter::sendMessage(const QString &message)
{
    try {
        auto convUid = LRCInstance::getCurrentConvUid();
        LRCInstance::getCurrentConversationModel()->sendMessage(convUid, message);
    } catch (...) {
        qDebug() << "Exception during sendMessage:" << message;
    }
}

void
MessagesAdapter::sendImage(const QString &message)
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
            auto convUid = LRCInstance::getCurrentConvUid();
            LRCInstance::getCurrentConversationModel()->sendFile(convUid, path, fileName);
        } catch (...) {
            qDebug().noquote() << "Exception during sendFile - base64 img"
                               << "\n";
        }

    } else {
        /*
         * Img tag contains file paths.
         */

        QString msg(message);
#ifdef Q_OS_WIN
        msg = msg.replace("file:///", "");
#else
        msg = msg.replace("file:///", "/");
#endif
        QFileInfo fi(msg);
        QString fileName = fi.fileName();

        try {
            auto convUid = LRCInstance::getCurrentConvUid();
            LRCInstance::getCurrentConversationModel()->sendFile(convUid, msg, fileName);
        } catch (...) {
            qDebug().noquote() << "Exception during sendFile - image from path"
                               << "\n";
        }
    }
}

void
MessagesAdapter::sendFile(const QString &message)
{
    QFileInfo fi(message);
    QString fileName = fi.fileName();
    try {
        auto convUid = LRCInstance::getCurrentConvUid();
        LRCInstance::getCurrentConversationModel()->sendFile(convUid, message, fileName);
    } catch (...) {
        qDebug() << "Exception during sendFile";
    }
}

void
MessagesAdapter::retryInteraction(const QString &arg)
{
    bool ok;
    uint64_t interactionUid = arg.toULongLong(&ok);
    if (ok) {
        LRCInstance::getCurrentConversationModel()
            ->retryInteraction(LRCInstance::getCurrentConvUid(), interactionUid);
    } else {
        qDebug() << "retryInteraction - invalid arg" << arg;
    }
}

void
MessagesAdapter::setNewMessagesContent(const QString &path)
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
MessagesAdapter::deleteInteraction(const QString &arg)
{
    bool ok;
    uint64_t interactionUid = arg.toULongLong(&ok);
    if (ok) {
        LRCInstance::getCurrentConversationModel()
            ->clearInteractionFromConversation(LRCInstance::getCurrentConvUid(), interactionUid);
    } else {
        qDebug() << "DeleteInteraction - invalid arg" << arg;
    }
}

void
MessagesAdapter::openFile(const QString &arg)
{
    QUrl fileUrl("file:///" + arg);
    if (!QDesktopServices::openUrl(fileUrl)) {
        qDebug() << "Couldn't open file: " << fileUrl;
    }
}

void
MessagesAdapter::acceptFile(const QString &arg)
{
    try {
        auto interactionUid = arg.toLongLong();
        auto convUid = LRCInstance::getCurrentConvUid();
        LRCInstance::getCurrentConversationModel()->acceptTransfer(convUid, interactionUid);
    } catch (...) {
        qDebug() << "JS bridging - exception during acceptFile: " << arg;
    }
}

void
MessagesAdapter::refuseFile(const QString &arg)
{
    try {
        auto interactionUid = arg.toLongLong();
        auto convUid = LRCInstance::getCurrentConvUid();
        LRCInstance::getCurrentConversationModel()->cancelTransfer(convUid, interactionUid);
    } catch (...) {
        qDebug() << "JS bridging - exception during refuseFile:" << arg;
    }
}

void
MessagesAdapter::pasteKeyDetected()
{
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();

    if (mimeData->hasImage()) {
        /*
         * Save temp data into base64 format.
         */
        QPixmap pixmap = qvariant_cast<QPixmap>(mimeData->imageData());
        QByteArray ba;
        QBuffer bu(&ba);
        bu.open(QIODevice::WriteOnly);
        pixmap.save(&bu, "PNG");
        auto str = QString::fromLocal8Bit(ba.toBase64());

        setMessagesImageContent(str, true);
    } else if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        /*
        * Extract the local paths of the files.
        */
        for (int i = 0; i < urlList.size(); ++i) {
            /*
             * Trim file:/// from url.
             */
            QString filePath = urlList.at(i).toString().remove(0, 8);
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
        QMetaObject::invokeMethod(qmlObj_,
                                  "webViewRunJavaScript",
                                  Q_ARG(QVariant,
                                        QStringLiteral("replaceText(`%1`)").arg(mimeData->text())));
    }
}

void
MessagesAdapter::onComposing(bool isComposing)
{
    LRCInstance::getCurrentConversationModel()->setIsComposing(LRCInstance::getCurrentConvUid(),
                                                               isComposing);
}

void
MessagesAdapter::setConversationProfileData(const lrc::api::conversation::Info &convInfo)
{
    auto convModel = LRCInstance::getCurrentConversationModel();
    auto accInfo = &LRCInstance::getCurrentAccountInfo();
    auto contactUri = convInfo.participants.front();

    if (contactUri.isEmpty()) {
        return;
    }
    try {
        auto &contact = accInfo->contactModel->getContact(contactUri);
        auto bestName = Utils::bestNameForConversation(convInfo, *convModel);
        setInvitation(contact.profileInfo.type == lrc::api::profile::Type::PENDING,
                      bestName,
                      contactUri);

        if (!contact.profileInfo.avatar.isEmpty()) {
            setSenderImage(contactUri, contact.profileInfo.avatar);
        } else {
            auto avatar = Utils::conversationPhoto(convInfo.uid, *accInfo, true);
            QByteArray ba;
            QBuffer bu(&ba);
            avatar.save(&bu, "PNG");
            setSenderImage(contactUri, QString::fromLocal8Bit(ba.toBase64()));
        }
    } catch (...) {
    }
}

void
MessagesAdapter::newInteraction(const QString &accountId,
                                const QString &convUid,
                                uint64_t interactionId,
                                const interaction::Info &interaction)
{
    Q_UNUSED(interactionId);
    try {
        auto &accountInfo = LRCInstance::getAccountInfo(accountId);
        auto &convModel = accountInfo.conversationModel;
        auto &conversation = LRCInstance::getConversationFromConvUid(convUid, accountId);

        if (conversation.uid.isEmpty()) {
            return;
        }
        if (!interaction.authorUri.isEmpty()
            && (!QApplication::focusWindow() || LRCInstance::getCurrAccId() != accountId)) {
            /*
             * TODO: Notification from other accounts.
             */
        }
        if (convUid != LRCInstance::getCurrentConvUid()) {
            return;
        }
        convModel->clearUnreadInteractions(convUid);
        printNewInteraction(*convModel, interactionId, interaction);
    } catch (...) {
    }
}

void
MessagesAdapter::updateDraft()
{
    Utils::oneShotConnect(qmlObj_,
                          SIGNAL(sendMessageContentSaved(const QString &)),
                          this,
                          SLOT(slotUpdateDraft(const QString &)));

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
MessagesAdapter::setInvitation(bool show, const QString &contactUri, const QString &contactId)
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
MessagesAdapter::printHistory(lrc::api::ConversationModel &conversationModel,
                              const std::map<uint64_t, lrc::api::interaction::Info> interactions)
{
    auto interactionsStr = interactionsToJsonArrayObject(conversationModel, interactions).toUtf8();
    QString s = QString::fromLatin1("printHistory(%1);").arg(interactionsStr.constData());
    QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
}

void
MessagesAdapter::setSenderImage(const QString &sender, const QString &senderImage)
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
MessagesAdapter::printNewInteraction(lrc::api::ConversationModel &conversationModel,
                                     uint64_t msgId,
                                     const lrc::api::interaction::Info &interaction)
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
MessagesAdapter::updateInteraction(lrc::api::ConversationModel &conversationModel,
                                   uint64_t msgId,
                                   const lrc::api::interaction::Info &interaction)
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
MessagesAdapter::setMessagesImageContent(const QString &path, bool isBased64)
{
    if (isBased64) {
        QString param = QString("addImage_base64('file://%1')").arg(path);
        QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, param));
    } else {
        QString param = QString("addImage_path('file://%1')").arg(path);
        QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, param));
    }
}

void
MessagesAdapter::setMessagesFileContent(const QString &path)
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
MessagesAdapter::setSendMessageContent(const QString &content)
{
    QString s = QString::fromLatin1("setSendMessageContent(`%1`);").arg(content);
    QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
}

void
MessagesAdapter::contactIsComposing(const QString &uid, const QString &contactUri, bool isComposing)
{
    if (LRCInstance::getCurrentConvUid() == uid) {
        QString s
            = QString::fromLatin1("showTypingIndicator(`%1`, %2);").arg(contactUri).arg(isComposing);
        QMetaObject::invokeMethod(qmlObj_, "webViewRunJavaScript", Q_ARG(QVariant, s));
    }
}

void
MessagesAdapter::acceptInvitation()
{
    auto convUid = LRCInstance::getCurrentConvUid();
    LRCInstance::getCurrentConversationModel()->makePermanent(convUid);
}

void
MessagesAdapter::refuseInvitation()
{
    auto convUid = LRCInstance::getCurrentConvUid();
    LRCInstance::getCurrentConversationModel()->removeConversation(convUid, false);
    setInvitation(false);
}

void
MessagesAdapter::blockConversation()
{
    auto convUid = LRCInstance::getCurrentConvUid();
    LRCInstance::getCurrentConversationModel()->removeConversation(convUid, true);
    setInvitation(false);
}
