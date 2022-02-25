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

#include <QMutex>

namespace lrc {
namespace video {

using namespace lrc::api::video;

class DirectRendererImpl_1 : public DirectRenderer
{
    Q_OBJECT
public:
    DirectRendererImpl_1(const QString& sinkId, const QSize& res)
    {
        configureTarget();
        VideoManager::instance().registerSinkTarget(id(), getTarget());
    };
    ~DirectRendererImpl_1()
    {
        stopRendering();
        VideoManager::instance().registerSinkTarget(id(), {});
    }

    DRing::SinkTarget& getTarget() override { return target; }

    // sink target callbacks
    void configureTarget()
    {
        using namespace std::placeholders;
        target.pull = std::bind(&DirectRendererImpl_1::pullCallback, this, _1);
        target.push = std::bind(&DirectRendererImpl_1::pushCallback, this, _1);
    };

    DRing::SinkTarget::FrameBufferPtr pullCallback(std::size_t bytes)
    {
        QMutexLocker lk(&mutex);
        if (!frameBufferPtr) {
            frameBufferPtr.reset(new DRing::FrameBuffer);
            assert(frameBufferPtr);
            frameBufferPtr->avframe.reset(av_frame_alloc());
        }

        auto& avframe = frameBufferPtr->avframe;
        assert(avframe);

        // A response to this signal should be used to provide client
        // allocated buffer specs via the AVFrame structure.
        // Important: Subscription to this signal MUST be synchronous(Qt::DirectConnection).
        Q_EMIT frameBufferRequested(avframe.get());

        // If no subscription to frameBufferRequested provided avFrame
        // internal buffer, then we provide one.
        if (avframe->data[0] == nullptr) {
            if (avframe->width > 0 and avframe->height > 0) {
                // Alignment is set to 0 to let the ffmpeg decides,
                // as recommended in the documentation.
                // TODO. Not thouroughly tested.
                if (av_frame_get_buffer(avframe.get(), 0) < 0) {
                    qCritical() << "Failed to allocate AV frame buffer";
                    return nullptr;
                }
            }
        }
        return std::move(frameBufferPtr);
    };

    void pushCallback(DRing::SinkTarget::FrameBufferPtr buf)
    {
        {
            QMutexLocker lk(&mutex);
            frameBufferPtr = std::move(buf);
        }

        Q_EMIT frameUpdated();
    };

public:
    DRing::SinkTarget target;
    QMutex mutex;
    DRing::SinkTarget::FrameBufferPtr frameBufferPtr;
};

DirectRenderer::DirectRenderer(const QString& id, const QSize& res)
    : Renderer(id, res)
//, pimpl_(std::make_unique<DirectRenderer::Impl>(this))
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
    Renderer::update(res);

    VideoManager::instance().registerSinkTarget(id(), getTarget());
}

Frame
DirectRenderer::currentFrame() const
{
    return {};
}

std::unique_ptr<DirectRenderer>
DirectRenderer::CreateInstance(const QString& id, const QSize& res)
{
    return std::move(std::make_unique<DirectRendererImpl_1>(id, res));
}

} // namespace video
} // namespace lrc

#include "moc_directrenderer.cpp"
#include "clientrenderer.moc"
