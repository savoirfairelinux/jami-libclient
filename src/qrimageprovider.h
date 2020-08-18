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

#include "accountlistmodel.h"
#include "lrcinstance.h"

#include <QImage>
#include <QObject>
#include <QPair>
#include <QQuickImageProvider>
#include <QString>

class QrImageProvider : public QObject, public QQuickImageProvider
{
public:
    QrImageProvider()
        : QQuickImageProvider(QQuickImageProvider::Image,
                              QQmlImageProviderBase::ForceAsynchronousImageLoading)
    {}

    enum class QrType { Account, Contact };

    /*
    * Id should be string like account_0 (account index),
    * or contact_xxx (uid).
    * Cannot use getCurrentAccId to replace account index,
    * since we need to keep each image id unique.
    */
    QPair<QrType, QString>
    getIndexFromID(const QString &id)
    {
        auto list = id.split('_', QString::SkipEmptyParts);
        if (list.contains("account")) {
            return QPair(QrType::Account, list[1]);
        } else if (list.contains("contact") && list.size() > 1) {
            /*
             * For contact_xxx, xxx is "" initially
             */

            auto convModel = LRCInstance::getCurrentConversationModel();
            auto convInfo = convModel->getConversationForUID(list[1]);
            auto contact = LRCInstance::getCurrentAccountInfo().contactModel->getContact(
                convInfo.participants.at(0));
            return QPair(QrType::Contact, contact.profileInfo.uri);
        }
        return QPair(QrType::Account, "");
    }

    QImage
    requestImage(const QString &id, QSize *size, const QSize &requestedSize) override
    {
        Q_UNUSED(size);

        QString uri;
        auto indexPair = getIndexFromID(id);

        if (indexPair.first == QrType::Contact) {
            uri = indexPair.second;
        } else {
            if (indexPair.second.isEmpty())
                return QImage();

            auto accountList = LRCInstance::accountModel().getAccountList();
            auto accountIndex = indexPair.second.toInt();
            if (accountList.size() <= accountIndex)
                return QImage();

            auto &accountInfo = LRCInstance::accountModel().getAccountInfo(
                accountList.at(accountIndex));
            uri = accountInfo.profileInfo.uri;
        }

        if (!requestedSize.isEmpty())
            return Utils::setupQRCode(uri, 0).scaled(requestedSize, Qt::KeepAspectRatio);
        else
            return Utils::setupQRCode(uri, 0);
    }
};
