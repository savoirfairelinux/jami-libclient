/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "distantrenderer.h"

#include "lrcinstance.h"

DistantRenderer::DistantRenderer(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setAntialiasing(true);
    setFillColor(Qt::black);
    setRenderTarget(QQuickPaintedItem::FramebufferObject);
    setPerformanceHint(QQuickPaintedItem::FastFBOResizing);

    connect(LRCInstance::renderer(), &RenderManager::distantFrameUpdated, [this](const QString& id) {
        if (distantRenderId_ == id)
            update(QRect(0, 0, width(), height()));
    });
}

DistantRenderer::~DistantRenderer() {}

void
DistantRenderer::setRendererId(const QString& id)
{
    distantRenderId_ = id;
}

int
DistantRenderer::getXOffset() const
{
    return xOffset_;
}

int
DistantRenderer::getYOffset() const
{
    return yOffset_;
}

double
DistantRenderer::getScaledWidth() const
{
    return scaledWidth_;
}

double
DistantRenderer::getScaledHeight() const
{
    return scaledHeight_;
}

void
DistantRenderer::paint(QPainter* painter)
{
    LRCInstance::renderer()->drawFrame(distantRenderId_, [this, painter](QImage* distantImage) {
        if (distantImage) {
            auto scaledDistant = distantImage->scaled(size().toSize(), Qt::KeepAspectRatio);
            auto tempScaledWidth = static_cast<int>(scaledWidth_ * 1000);
            auto tempScaledHeight = static_cast<int>(scaledHeight_ * 1000);
            auto tempXOffset = xOffset_;
            auto tempYOffset = yOffset_;
            scaledWidth_ = static_cast<double>(scaledDistant.width())
                           / static_cast<double>(distantImage->width());
            scaledHeight_ = static_cast<double>(scaledDistant.height())
                            / static_cast<double>(distantImage->height());
            xOffset_ = (width() - scaledDistant.width()) / 2;
            yOffset_ = (height() - scaledDistant.height()) / 2;
            if (tempXOffset != xOffset_ or tempYOffset != yOffset_
                or static_cast<int>(scaledWidth_ * 1000) != tempScaledWidth
                or static_cast<int>(scaledHeight_ * 1000) != tempScaledHeight) {
                emit offsetChanged();
            }
            painter->drawImage(QRect(xOffset_,
                                     yOffset_,
                                     scaledDistant.width(),
                                     scaledDistant.height()),
                               scaledDistant);
        }
    });
}
