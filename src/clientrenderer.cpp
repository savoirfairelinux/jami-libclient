/*
 *  Copyright (C) 2012-2022 Savoir-faire Linux Inc.
 *
 *  Author: Alexandre Lision <alexandre.lision@savoirfairelinux.com>
 *  Author: Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "directrenderer.h"

#include "dbus/videomanager.h"
#include "videomanager_interface.h"

#include <QMutex>

namespace lrc {
namespace video {

using namespace lrc::api::video;

struct DirectRenderer::Impl : public QObject
{
    Q_OBJECT
public:
    Impl(DirectRenderer* parent)
        : QObject(nullptr)
        , parent_(parent)
        , frame {nullptr, AVFrameDeleter}
    {
        configureTarget();
        VideoManager::instance().registerSinkTarget(parent_->id(), target);
    };
    ~Impl()
    {
        parent_->stopRendering();
        VideoManager::instance().registerSinkTarget(parent_->id(), {});
    }

    // sink target callbacks
    void configureTarget()
    {
        using namespace std::placeholders;
        target.pull = std::bind(&Impl::pullCallback, this, _1);
        target.push = std::bind(&Impl::pushCallback, this, _1);
    };

    DRing::SinkTarget::FrameBufferPtr pullCallback(std::size_t bytes)
    {
        QMutexLocker lk(&mutex);
        if (!frameBufferPtr) {
            frameBufferPtr.reset(new DRing::FrameBuffer);
            frameBufferPtr->avframe.reset(av_frame_alloc());
        }

        // A response to this signal should be used to provide client
        // allocated buffer specs via the AVFrame structure.
        // Important: Subscription to this signal MUST be synchronous(Qt::DirectConnection).
        Q_EMIT parent_->frameBufferRequested(frameBufferPtr->avframe.get());

        // If no subscription to frameBufferRequested filled avFrame, then
        // we revert to legacy storage and the use of currentFrame.
        if (frameBufferPtr->avframe->data[0] == nullptr) {
            frameBufferPtr->storage.resize(bytes);
            frameBufferPtr->ptr = frameBufferPtr->storage.data();
            frameBufferPtr->ptrSize = bytes;
        }

        return std::move(frameBufferPtr);
    };

    void pushCallback(DRing::SinkTarget::FrameBufferPtr buf)
    {
        {
            QMutexLocker lk(&mutex);
            frameBufferPtr = std::move(buf);
        }

        Q_EMIT parent_->frameUpdated();
    };

    DRing::SinkTarget target;
    QMutex mutex;
    DRing::SinkTarget::FrameBufferPtr frameBufferPtr;
    Frame frame;

private:
    DirectRenderer* parent_;
};

DirectRenderer::DirectRenderer(const QString& id, const QSize& res)
    : Renderer(id, res)
    , pimpl_(std::make_unique<DirectRenderer::Impl>(this))
{}

DirectRenderer::~DirectRenderer()
{
    QMutexLocker lk(&pimpl_->mutex);
    stopRendering();
}

void
DirectRenderer::startRendering()
{
    Q_EMIT started();
}

void
DirectRenderer::stopRendering()
{
    Q_EMIT stopped();
}

void
DirectRenderer::update(const QSize& res, const QString&)
{
    Renderer::update(res);
    VideoManager::instance().registerSinkTarget(id(), pimpl_->target);
}

Frame
DirectRenderer::currentFrame() const
{
    QMutexLocker lock(&pimpl_->mutex);
    if (not pimpl_->frameBufferPtr)
        return {nullptr, AVFrameDeleter};

    return std::move(pimpl_->frame);
}

} // namespace video
} // namespace lrc

#include "moc_directrenderer.cpp"
#include "clientrenderer.moc"
