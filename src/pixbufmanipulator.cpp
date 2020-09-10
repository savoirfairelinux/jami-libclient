/*
 * Copyright (C) 2015-2020 by Savoir-faire Linux
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>
 * Author: Anthony Léonard <anthony.leonard@savoirfairelinux.com>
 * Author: Olivier Soldano <olivier.soldano@savoirfairelinux.com>
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "pixbufmanipulator.h"

#include <QBuffer>
#include <QByteArray>
#include <QIODevice>
#include <QImage>
#include <QMetaType>
#include <QPainter>
#include <QSize>

#include "globalinstances.h"

#include <api/account.h>
#include <api/contact.h>
#include <api/contactmodel.h>
#include <api/conversation.h>

#include "utils.h"
#undef interface

QVariant
PixbufManipulator::personPhoto(const QByteArray& data, const QString& type)
{
    QImage avatar;
    const bool ret = avatar.loadFromData(QByteArray::fromBase64(data), type.toLatin1());
    if (!ret) {
        qDebug() << "vCard image loading failed";
        return QVariant();
    }
    return QPixmap::fromImage(Utils::getCirclePhoto(avatar, avatar.size().width()));
}

QVariant
PixbufManipulator::numberCategoryIcon(const QVariant& p,
                                      const QSize& size,
                                      bool displayPresence,
                                      bool isPresent)
{
    Q_UNUSED(p)
    Q_UNUSED(size)
    Q_UNUSED(displayPresence)
    Q_UNUSED(isPresent)
    return QVariant();
}

QByteArray
PixbufManipulator::toByteArray(const QVariant& pxm)
{
    auto image = pxm.value<QImage>();
    QByteArray ba = Utils::QImageToByteArray(image);
    return ba;
}

QVariant
PixbufManipulator::userActionIcon(const UserActionElement& state) const
{
    Q_UNUSED(state)
    return QVariant();
}

QVariant
PixbufManipulator::decorationRole(const QModelIndex& index)
{
    Q_UNUSED(index)
    return QVariant();
}

QVariant
PixbufManipulator::decorationRole(const lrc::api::conversation::Info& conversationInfo,
                                  const lrc::api::account::Info& accountInfo)
{
    QImage photo;
    auto contacts = conversationInfo.participants;
    if (contacts.empty()) {
        return QVariant::fromValue(photo);
    }
    try {
        /*
         * Get first contact photo.
         */
        auto contactUri = contacts.front();
        auto contactInfo = accountInfo.contactModel->getContact(contactUri);
        auto contactPhoto = contactInfo.profileInfo.avatar;
        auto bestName = Utils::bestNameForContact(contactInfo);
        auto bestId = Utils::bestIdForContact(contactInfo);
        if (accountInfo.profileInfo.type == lrc::api::profile::Type::SIP
            && contactInfo.profileInfo.type == lrc::api::profile::Type::TEMPORARY) {
            photo = Utils::fallbackAvatar(QString(), QString());
        } else if (contactInfo.profileInfo.type == lrc::api::profile::Type::TEMPORARY
                   && contactInfo.profileInfo.uri.isEmpty()) {
            photo = Utils::fallbackAvatar(QString(), QString());
        } else if (!contactPhoto.isEmpty()) {
            QByteArray byteArray = contactPhoto.toLocal8Bit();
            photo = personPhoto(byteArray, nullptr).value<QImage>();
            if (photo.isNull()) {
                auto avatarName = contactInfo.profileInfo.uri == bestName ? QString() : bestName;
                photo = Utils::fallbackAvatar("ring:" + contactInfo.profileInfo.uri, avatarName);
            }
        } else {
            auto avatarName = contactInfo.profileInfo.uri == bestName ? QString() : bestName;
            photo = Utils::fallbackAvatar("ring:" + contactInfo.profileInfo.uri, avatarName);
        }
    } catch (...) {
    }
    return QVariant::fromValue(Utils::scaleAndFrame(photo));
}
