/*
 * Copyright (C) 2020 by Savoir-faire Linux
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "utils.h"

#include <QImage>
#include <QQuickImageProvider>

class AvatarImageProvider : public QObject, public QQuickImageProvider
{
public:
    AvatarImageProvider()
        : QQuickImageProvider(QQuickImageProvider::Image,
                              QQmlImageProviderBase::ForceAsynchronousImageLoading)
    {}

    /*
     * Request function
     * id could be
     * 1. account_ + account id
     * 2. file_ + file path
     * 3. contact_+ contact uri
     * 4. conversation_+ conversation uid
     */
    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override
    {
        Q_UNUSED(size)

        auto idInfo = id.split("_");
        // Id type -> account_
        auto idType = idInfo[1];
        // Id content -> every after account_
        auto idContent = id.mid(id.indexOf(idType) + idType.length() + 1);

        if (idContent.isEmpty())
            return QImage();

        if (idType == "account") {
            return Utils::accountPhoto(LRCInstance::accountModel().getAccountInfo(idContent),
                                       requestedSize);
        } else if (idType == "conversation") {
            auto* convModel = LRCInstance::getCurrentAccountInfo().conversationModel.get();
            const auto& conv = convModel->getConversationForUID(idContent);
            return Utils::contactPhoto(conv.participants[0], requestedSize);
        } else if (idType == "contact") {
            return Utils::contactPhoto(idContent, requestedSize);
        } else {
            auto image = Utils::cropImage(QImage(idContent));
            return image.scaled(requestedSize,
                                Qt::KeepAspectRatioByExpanding,
                                Qt::SmoothTransformation);
        }
    }
};
