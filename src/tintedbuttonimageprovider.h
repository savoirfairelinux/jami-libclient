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
#include "lrcinstance.h"
#include "utils.h"

#include <QPair>
#include <QString>

class TintedButtonImageProvider : public QuickImageProviderBase
{
public:
    TintedButtonImageProvider(LRCInstance* instance = nullptr)
        : QuickImageProviderBase(QQuickImageProvider::Pixmap,
                                  QQmlImageProviderBase::ForceAsynchronousImageLoading,
                                  instance)
    {}

    QPixmap requestPixmap(const QString& id, QSize* size, const QSize& requestedSize) override
    {
        Q_UNUSED(size);

        QColor tintedColor;

        auto list = id.split('+', QString::SkipEmptyParts);

        if (list.size() == 2) {
            QPixmap pixmapToSend(":/images/icons/" + list[0]);
            if (!requestedSize.isEmpty()) {
                pixmapToSend = pixmapToSend.scaled(requestedSize, Qt::KeepAspectRatio);
            } else {
                pixmapToSend = pixmapToSend.scaled(QSize(30, 30), Qt::KeepAspectRatio);
            }
            tintedColor.setNamedColor(list[1]);

            return Utils::generateTintedPixmap(pixmapToSend, tintedColor);
        }

        return QPixmap();
    }
};
