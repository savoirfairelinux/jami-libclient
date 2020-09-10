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

/*
 * Use QQuickPaintedItem so that QPainter apis can be used.
 * Note: Old video pipeline.
 */
class PreviewRenderer : public QQuickPaintedItem
{
    Q_OBJECT
public:
    explicit PreviewRenderer(QQuickItem* parent = 0);
    ~PreviewRenderer();

protected:
    void paint(QPainter* painter) override;

private:
    QMetaObject::Connection previewFrameUpdatedConnection_;
    QMetaObject::Connection previewRenderingStopped_;
};

class VideoCallPreviewRenderer : public PreviewRenderer
{
    Q_OBJECT
public:
    explicit VideoCallPreviewRenderer(QQuickItem* parent = 0);
    virtual ~VideoCallPreviewRenderer();

    Q_INVOKABLE qreal getPreviewImageScalingFactor();

signals:
    void previewImageAvailable();

private:
    void paint(QPainter* painter) override final;
};

class PhotoboothPreviewRender : public PreviewRenderer
{
    Q_OBJECT
public:
    explicit PhotoboothPreviewRender(QQuickItem* parent = 0);
    virtual ~PhotoboothPreviewRender();

    QImage takePhoto();
    Q_INVOKABLE QString takeCroppedPhotoToBase64(int size);

signals:
    void hideBooth();

private:
    void paint(QPainter* painter) override final;
};
