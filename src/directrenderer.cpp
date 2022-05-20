/*
 *  Copyright (C) 2012-2022 Savoir-faire Linux Inc.
 *  Author: Alexandre Lision <alexandre.lision@savoirfairelinux.com>
 *  Author: Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>
 *  Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
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
        target.pull = std::bind(&Impl::pullCallback, this);
        target.push = std::bind(&Impl::pushCallback, this, _1);
    };

    DRing::FrameBuffer pullCallback()
    {
        QMutexLocker lk(&mutex);
        if (!frameBufferPtr) {
            frameBufferPtr.reset(av_frame_alloc());
        }

        // A response to this signal should be used to provide client
        // allocated buffer specs via the AVFrame structure.
        // Important: Subscription to this signal MUST be synchronous(Qt::DirectConnection).
        Q_EMIT parent_->frameBufferRequested(frameBufferPtr.get());

        if (frameBufferPtr->data[0] == nullptr) {
            return nullptr;
        }

        return std::move(frameBufferPtr);
    };

    void pushCallback(DRing::FrameBuffer buf)
    {
        {
            QMutexLocker lk(&mutex);
            frameBufferPtr = std::move(buf);
        }

        Q_EMIT parent_->frameUpdated();
    };

private:
    DirectRenderer* parent_;

public:
    DRing::SinkTarget target;
    QMutex mutex;
    DRing::FrameBuffer frameBufferPtr;
};

DirectRenderer::DirectRenderer(const QString& id, const QSize& res)
    : Renderer(id, res)
    , pimpl_(std::make_unique<DirectRenderer::Impl>(this))
{}

DirectRenderer::~DirectRenderer() {}

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
    stopRendering();
    Renderer::update(res);

    VideoManager::instance().registerSinkTarget(id(), pimpl_->target);
    startRendering();
}

Frame
DirectRenderer::currentFrame() const
{
    return {};
}

} // namespace video
} // namespace lrc

#include "moc_directrenderer.cpp"
#include "directrenderer.moc"
