/*
 * Copyright (C) 2015-2020 by Savoir-faire Linux
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>
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

#include <QImage>

#include <interfaces/pixmapmanipulatori.h>
#include <memory>

Q_DECLARE_METATYPE(QImage);

class Person;

QByteArray QImageToByteArray(QImage image);

class PixbufManipulator : public Interfaces::PixmapManipulatorI
{
public:
    QVariant personPhoto(const QByteArray& data, const QString& type = "PNG") override;

    /*
     * TODO: the following methods return an empty QVariant/QByteArray.
     */
    QVariant numberCategoryIcon(const QVariant& p,
                                const QSize& size,
                                bool displayPresence = false,
                                bool isPresent = false) override;
    QByteArray toByteArray(const QVariant& pxm) override;
    QVariant userActionIcon(const UserActionElement& state) const override;
    QVariant decorationRole(const QModelIndex& index) override;
    QVariant decorationRole(const lrc::api::conversation::Info& conversation,
                            const lrc::api::account::Info& accountInfo) override;
};
