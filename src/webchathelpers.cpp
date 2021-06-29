/*
 * Copyright (C) 2017-2020 by Savoir-faire Linux
 * Author: Alexandre Viau <alexandre.viau@savoirfairelinux.com>
 * Author: S�bastien Blin <sebastien.blin@savoirfairelinux.com>
 * Author: Hugo Lefeuvre <hugo.lefeuvre@savoirfairelinux.com>
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

#include "webchathelpers.h"

QJsonObject
buildInteractionJson(lrc::api::ConversationModel& conversationModel,
                     const QString& convId,
                     const QString msgId,
                     const lrc::api::interaction::Info& inter)
{
    QRegExp reg(".(jpeg|jpg|gif|png)$");
    auto interaction = inter;
    if (interaction.type == lrc::api::interaction::Type::DATA_TRANSFER) {
        if (interaction.body.isEmpty())
            return {};
        else if (interaction.body.toLower().contains(reg))
            interaction.body = "file://" + interaction.body;
    }

    if (interaction.type == lrc::api::interaction::Type::MERGE)
        return {};

    auto sender = interaction.authorUri;
    auto timestamp = QString::number(interaction.timestamp);
    auto direction = lrc::api::interaction::isOutgoing(interaction) ? QString("out")
                                                                    : QString("in");

    QJsonObject interactionObject = QJsonObject();
    interactionObject.insert("text", QJsonValue(interaction.body));
    interactionObject.insert("id", QJsonValue(msgId));
    interactionObject.insert("sender", QJsonValue(sender));
    interactionObject.insert("sender_contact_method", QJsonValue(sender));
    interactionObject.insert("timestamp", QJsonValue(timestamp));
    interactionObject.insert("direction", QJsonValue(direction));
    interactionObject.insert("duration", QJsonValue(static_cast<int>(interaction.duration)));

    switch (interaction.type) {
    case lrc::api::interaction::Type::TEXT:
        interactionObject.insert("type", QJsonValue("text"));
        break;
    case lrc::api::interaction::Type::CALL:
        interactionObject.insert("type", QJsonValue("call"));
        break;
    case lrc::api::interaction::Type::CONTACT:
        interactionObject.insert("type", QJsonValue("contact"));
        break;
    case lrc::api::interaction::Type::DATA_TRANSFER: {
        interactionObject.insert("type", QJsonValue("data_transfer"));
        lrc::api::datatransfer::Info info = {};
        conversationModel.getTransferInfo(convId, msgId, info);
        if (info.status != lrc::api::datatransfer::Status::INVALID) {
            interactionObject.insert("totalSize", QJsonValue(qint64(info.totalSize)));
            interactionObject.insert("progress", QJsonValue(qint64(info.progress)));
        }
        interactionObject.insert("displayName", QJsonValue(inter.commit["displayName"]));
        break;
    }
    case lrc::api::interaction::Type::INVALID:
    default:
        return {};
    }

    if (interaction.isRead) {
        interactionObject.insert("delivery_status", QJsonValue("read"));
    }

    switch (interaction.status) {
    case lrc::api::interaction::Status::SUCCESS:
        interactionObject.insert("delivery_status", QJsonValue("sent"));
        break;
    case lrc::api::interaction::Status::FAILURE:
    case lrc::api::interaction::Status::TRANSFER_ERROR:
        interactionObject.insert("delivery_status", QJsonValue("failure"));
        break;
    case lrc::api::interaction::Status::TRANSFER_UNJOINABLE_PEER:
        interactionObject.insert("delivery_status", QJsonValue("unjoinable peer"));
        break;
    case lrc::api::interaction::Status::SENDING:
        interactionObject.insert("delivery_status", QJsonValue("sending"));
        break;
    case lrc::api::interaction::Status::TRANSFER_CREATED:
        interactionObject.insert("delivery_status", QJsonValue("connecting"));
        break;
    case lrc::api::interaction::Status::TRANSFER_ACCEPTED:
        interactionObject.insert("delivery_status", QJsonValue("accepted"));
        break;
    case lrc::api::interaction::Status::TRANSFER_CANCELED:
        interactionObject.insert("delivery_status", QJsonValue("canceled"));
        break;
    case lrc::api::interaction::Status::TRANSFER_ONGOING:
        interactionObject.insert("delivery_status", QJsonValue("ongoing"));
        break;
    case lrc::api::interaction::Status::TRANSFER_AWAITING_PEER:
        interactionObject.insert("delivery_status", QJsonValue("awaiting peer"));
        break;
    case lrc::api::interaction::Status::TRANSFER_AWAITING_HOST:
        interactionObject.insert("delivery_status", QJsonValue("awaiting host"));
        break;
    case lrc::api::interaction::Status::TRANSFER_TIMEOUT_EXPIRED:
        interactionObject.insert("delivery_status", QJsonValue("awaiting peer timeout"));
        break;
    case lrc::api::interaction::Status::TRANSFER_FINISHED:
        interactionObject.insert("delivery_status", QJsonValue("finished"));
        break;
    case lrc::api::interaction::Status::INVALID:
    case lrc::api::interaction::Status::UNKNOWN:
    default:
        interactionObject.insert("delivery_status", QJsonValue("unknown"));
        break;
    }
    return interactionObject;
}

QString
interactionToJsonInteractionObject(lrc::api::ConversationModel& conversationModel,
                                   const QString& convId,
                                   const QString& msgId,
                                   const lrc::api::interaction::Info& interaction)
{
    auto interactionObject = buildInteractionJson(conversationModel, convId, msgId, interaction);
    return QString(QJsonDocument(interactionObject).toJson(QJsonDocument::Compact));
}

QString
interactionsToJsonArrayObject(lrc::api::ConversationModel& conversationModel,
                              const QString& convId,
                              MessagesList interactions)
{
    QJsonArray array;
    for (const auto& interaction : interactions) {
        auto interactionObject = buildInteractionJson(conversationModel,
                                                      convId,
                                                      interaction.first,
                                                      interaction.second);
        if (!interactionObject.isEmpty()) {
            array.append(interactionObject);
        }
    }
    return QString(QJsonDocument(array).toJson(QJsonDocument::Compact));
}
