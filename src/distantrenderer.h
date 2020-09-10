/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 * Author: Mingrui Zhang   <mingrui.zhang@savoirfairelinux.com>
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

#pragma once

#include <QtQuick>

/*
 * Use QQuickPaintedItem so that QPainter apis can be used.
 * Note: Old video pipeline.
 */

class DistantRenderer : public QQuickPaintedItem
{
    Q_OBJECT
public:
    explicit DistantRenderer(QQuickItem* parent = 0);
    ~DistantRenderer();

    Q_INVOKABLE void setRendererId(const QString& id);
    Q_INVOKABLE int getXOffset() const;
    Q_INVOKABLE int getYOffset() const;
    Q_INVOKABLE double getScaledWidth() const;
    Q_INVOKABLE double getScaledHeight() const;

signals:
    void offsetChanged();

private:
    void paint(QPainter* painter);

    /*
     * Unique DistantRenderId for each call.
     */
    QString distantRenderId_;
    int xOffset_ {0};
    int yOffset_ {0};
    double scaledWidth_ {0};
    double scaledHeight_ {0};
};