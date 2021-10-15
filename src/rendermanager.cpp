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

#include "rendermanager.h"

#include <stdexcept>

using namespace lrc::api;

FrameWrapper::FrameWrapper(AVModel& avModel, const QString& id)
    : avModel_(avModel)
    , id_(id)
    , isRendering_(false)
{}

FrameWrapper::~FrameWrapper()
{
    avModel_.stopPreview(id_);
}

void
FrameWrapper::connectStartRendering()
{
    QObject::disconnect(renderConnections_.started);
    renderConnections_.started = QObject::connect(&avModel_,
                                                  &AVModel::rendererStarted,
                                                  this,
                                                  &FrameWrapper::slotRenderingStarted);
}

bool
FrameWrapper::startRendering()
{
    if (isRendering())
        return true;

    try {
        renderer_ = const_cast<video::Renderer*>(&avModel_.getRenderer(id_));
    } catch (std::out_of_range& e) {
        qWarning() << e.what();
        return false;
    }

    QObject::disconnect(renderConnections_.updated);
    QObject::disconnect(renderConnections_.stopped);

    renderConnections_.updated = QObject::connect(&avModel_,
                                                  &AVModel::frameUpdated,
                                                  this,
                                                  &FrameWrapper::slotFrameUpdated);

    renderConnections_.stopped = QObject::connect(&avModel_,
                                                  &AVModel::rendererStopped,
                                                  this,
                                                  &FrameWrapper::slotRenderingStopped,
                                                  Qt::DirectConnection);

    return true;
}

void
FrameWrapper::stopRendering()
{
    isRendering_ = false;
}

QImage*
FrameWrapper::getFrame()
{
    if (image_.get()) {
        return isRendering_ ? (image_.get()->isNull() ? nullptr : image_.get()) : nullptr;
    }
    return nullptr;
}

bool
FrameWrapper::isRendering()
{
    return isRendering_;
}

bool
FrameWrapper::frameMutexTryLock()
{
    return mutex_.tryLock();
}

void
FrameWrapper::frameMutexUnlock()
{
    mutex_.unlock();
}

void
FrameWrapper::slotRenderingStarted(const QString& id)
{
    if (id != id_) {
        return;
    }

    if (!startRendering()) {
        qWarning() << "Couldn't start rendering for id: " << id_;
        return;
    }

    isRendering_ = true;

    Q_EMIT renderingStarted(id);
}

void
FrameWrapper::slotFrameUpdated(const QString& id)
{
    if (id != id_) {
        return;
    }

    if (!renderer_ || !renderer_->isRendering()) {
        return;
    }

    {
        QMutexLocker lock(&mutex_);

        frame_ = renderer_->currentFrame();

        unsigned int width = renderer_->size().width();
        unsigned int height = renderer_->size().height();
        unsigned int size;
        QImage::Format imageFormat;
        if (renderer_->useDirectRenderer()) {
            size = frame_.storage.size();
            imageFormat = QImage::Format_ARGB32_Premultiplied;
        } else {
            size = frame_.size;
            imageFormat = QImage::Format_ARGB32;
        }
        /*
         * If the frame is empty or not the expected size,
         * do nothing and keep the last rendered QImage.
         */
        if (size != 0 && size == width * height * 4) {
            if (renderer_->useDirectRenderer()) {
                buffer_ = std::move(frame_.storage);
            } else {
                // TODO remove this path. storage should work everywhere
                // https://git.jami.net/savoirfairelinux/jami-libclient/-/issues/492
                buffer_.resize(size);
                std::move(frame_.ptr, frame_.ptr + size, buffer_.begin());
            }
            image_.reset(new QImage((uchar*) buffer_.data(), width, height, imageFormat));
        }
    }
    Q_EMIT frameUpdated(id);
}

void
FrameWrapper::slotRenderingStopped(const QString& id)
{
    if (id != id_) {
        return;
    }
    isRendering_ = false;

    QObject::disconnect(renderConnections_.updated);

    renderer_ = nullptr;

    {
        QMutexLocker lock(&mutex_);
        image_.reset();
    }

    Q_EMIT renderingStopped(id);
}

RenderManager::RenderManager(AVModel& avModel)
    : avModel_(avModel)
{}

RenderManager::~RenderManager()
{
    for (auto& dfw : distantFrameWrapperMap_) {
        dfw.second.reset();
    }
}

void
RenderManager::stopPreviewing(const QString& id)
{
    auto dfwIt = distantFrameWrapperMap_.find(id);
    if (dfwIt != distantFrameWrapperMap_.end()) {
        dfwIt->second->stopRendering();
        avModel_.stopPreview(id);
    }
}

const QString
RenderManager::startPreviewing(const QString& id, bool force)
{
    auto dfwIt = distantFrameWrapperMap_.find(id);
    if (dfwIt != distantFrameWrapperMap_.end()) {
        if (dfwIt->second->isRendering() && !force) {
            return dfwIt->second->getId();
        }

        if (dfwIt->second->isRendering()) {
            avModel_.stopPreview(id);
        }
        return avModel_.startPreview(id);
    }
    return "";
}

void
RenderManager::addDistantRenderer(const QString& id)
{
    /*
     * Check if a FrameWrapper with this id exists.
     */
    auto dfwIt = distantFrameWrapperMap_.find(id);
    if (dfwIt != distantFrameWrapperMap_.end()) {
        if (!dfwIt->second->startRendering()) {
            qWarning() << "Couldn't start rendering for id: " << id;
        }
    } else {
        auto dfw = std::make_unique<FrameWrapper>(avModel_, id);

        /*
         * Connect this to the FrameWrapper.
         */
        distantConnectionMap_[id].updated = QObject::connect(dfw.get(),
                                                             &FrameWrapper::frameUpdated,
                                                             [this](const QString& id) {
                                                                 Q_EMIT distantFrameUpdated(id);
                                                             });
        distantConnectionMap_[id].stopped = QObject::connect(dfw.get(),
                                                             &FrameWrapper::renderingStopped,
                                                             [this](const QString& id) {
                                                                 Q_EMIT distantRenderingStopped(id);
                                                             });

        /*
         * Connect FrameWrapper to avmodel.
         */
        dfw->connectStartRendering();
        try {
            /*
             * If the renderer has already started, then start the slot.
             */
            if (avModel_.getRenderer(id).isRendering())
                dfw->slotRenderingStarted(id);
        } catch (...) {
        }

        /*
         * Add to map.
         */
        distantFrameWrapperMap_.insert(std::make_pair(id, std::move(dfw)));
    }
}

void
RenderManager::removeDistantRenderer(const QString& id)
{
    auto dfwIt = distantFrameWrapperMap_.find(id);
    if (dfwIt != distantFrameWrapperMap_.end()) {
        /*
         * Disconnect FrameWrapper from this.
         */
        auto dcIt = distantConnectionMap_.find(id);
        if (dcIt != distantConnectionMap_.end()) {
            QObject::disconnect(dcIt->second.started);
            QObject::disconnect(dcIt->second.updated);
            QObject::disconnect(dcIt->second.stopped);
        }

        /*
         * Erase.
         */
        distantFrameWrapperMap_.erase(dfwIt);
    }
}

void
RenderManager::drawFrame(const QString& id, DrawFrameCallback cb)
{
    auto dfwIt = distantFrameWrapperMap_.find(id);
    if (dfwIt != distantFrameWrapperMap_.end()) {
        if (dfwIt->second->frameMutexTryLock()) {
            cb(dfwIt->second->getFrame());
            dfwIt->second->frameMutexUnlock();
        }
    }
}

QImage*
RenderManager::getPreviewFrame(const QString& id)
{
    auto dfwIt = distantFrameWrapperMap_.find(id);
    if (dfwIt != distantFrameWrapperMap_.end()) {
        return dfwIt->second->getFrame();
    }
    return {};
}
