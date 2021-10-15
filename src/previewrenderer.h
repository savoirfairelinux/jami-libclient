/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
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

#include <QtQuick>

#include "lrcinstance.h"

/*
 * Use QQuickPaintedItem so that QPainter apis can be used.
 * Note: Old video pipeline.
 */
class PreviewRenderer : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(LRCInstance* lrcInstance MEMBER lrcInstance_ NOTIFY lrcInstanceChanged)
    QML_PROPERTY(QString, rendererId);

public:
    explicit PreviewRenderer(QQuickItem* parent = nullptr);
    virtual ~PreviewRenderer();

Q_SIGNALS:
    void lrcInstanceChanged();

protected:
    void paint(QPainter* painter) override;
    void paintBackground(QPainter* painter);

    // LRCInstance pointer (set in qml)
    LRCInstance* lrcInstance_ {nullptr};

private:
    QMetaObject::Connection previewFrameUpdatedConnection_;
};

class VideoCallPreviewRenderer : public PreviewRenderer
{
    Q_OBJECT
    Q_PROPERTY(qreal previewImageScalingFactor MEMBER previewImageScalingFactor_ NOTIFY
                   previewImageScalingFactorChanged)
public:
    explicit VideoCallPreviewRenderer(QQuickItem* parent = nullptr);
    ~VideoCallPreviewRenderer();

Q_SIGNALS:
    void previewImageScalingFactorChanged();

private:
    void paint(QPainter* painter) override final;

    qreal previewImageScalingFactor_;
};

class PhotoboothPreviewRender : public PreviewRenderer
{
    Q_OBJECT
public:
    explicit PhotoboothPreviewRender(QQuickItem* parent = nullptr);
    ~PhotoboothPreviewRender() = default;

    Q_INVOKABLE QString takePhoto(int size);

Q_SIGNALS:
    void renderingStopped(const QString id);

private:
    void paint(QPainter* painter) override final;
};
