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

#include "quickimageproviderbase.h"
#include "utils.h"
#include "lrcinstance.h"

#include <QImage>

class AvatarImageProvider : public QuickImageProviderBase
{
public:
    AvatarImageProvider(LRCInstance* instance = nullptr)
        : QuickImageProviderBase(QQuickImageProvider::Image,
                                 QQmlImageProviderBase::ForceAsynchronousImageLoading,
                                 instance)
    {}

    /*
     * Request function
     * id could be
     * 1. account_ + account id
     * 2. file_ + file path
     * 3. contact_+ contact uri
     * 4. conversation_+ conversation uid
     * 5. base64_ + base64 string
     */
    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override
    {
        Q_UNUSED(size)

        auto idInfo = id.split("_");
        // Id type -> account_
        auto idType = idInfo[1];
        // Id content -> every after account_
        auto idContent = id.mid(id.indexOf(idType) + idType.length() + 1);

        if (idContent.isEmpty() && idType != "default")
            return QImage();

        if (idType == "account") {
            return Utils::accountPhoto(lrcInstance_,
                                       lrcInstance_->accountModel().getAccountInfo(idContent),
                                       requestedSize);
        } else if (idType == "conversation") {
            const auto& convInfo = lrcInstance_->getConversationFromConvUid(idContent);
            return Utils::contactPhoto(lrcInstance_, convInfo.participants[0], requestedSize);
        } else if (idType == "contact") {
            return Utils::contactPhoto(lrcInstance_, idContent, requestedSize);
        } else if (idType == "fallback") {
            return Utils::fallbackAvatar(idContent, QString(), requestedSize);
        } else if (idType == "default") {
            return Utils::fallbackAvatar(QString(), QString(), requestedSize);
        } else if (idType == "base64") {
            return Utils::cropImage(QImage::fromData(Utils::base64StringToByteArray(idContent)))
                .scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        } else {
            QImage image = QImage(idContent);
            return Utils::getCirclePhoto(image, image.size().width())
                .scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }
};
