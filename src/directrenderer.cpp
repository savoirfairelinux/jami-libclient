/****************************************************************************
 *   Copyright (C) 2012-2022 Savoir-faire Linux Inc.                        *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com>      *
 *   Author : Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>      *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Lesser General Public             *
 *   License as published by the Free Software Foundation; either           *
 *   version 2.1 of the License, or (at your option) any later version.     *
 *                                                                          *
 *   This library is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/
#ifdef ENABLE_LIBWRAP

#include "directrenderer.h"

#include <QtCore/QDebug>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QTime>
#include <QtCore/QTimer>

#include <cstring>

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif

#include "private/videorenderer_p.h"
#include "videomanager_interface.h"

extern "C" {
struct AVFrame;
auto AVFrameDeleter = [](AVFrame* p) {
};
}

namespace Video {
class DirectRendererPrivate : public QObject
{
    Q_OBJECT
public:
    DirectRendererPrivate(Video::DirectRenderer* parent, bool useAVFrame);
    DRing::SinkTarget::FrameBufferPtr pullFrameBuffer(std::size_t bytes);
    void pushFrameBuffer(DRing::SinkTarget::FrameBufferPtr buf);
    void pushAVFrameBuffer(std::unique_ptr<DRing::VideoFrame> frame);
    void configureTarget(bool useAVFrame);

    DRing::SinkTarget sinkTarget_;
    DRing::AVSinkTarget av_target;
    mutable DRing::SinkTarget::FrameBufferPtr daemonFramePtr_;
    std::unique_ptr<AVFrame, void (*)(AVFrame*)> avframe;
    size_t frameCounter_ {0};

private:
    Video::DirectRenderer* q_ptr;
};

} // namespace Video

Video::DirectRendererPrivate::DirectRendererPrivate(Video::DirectRenderer* parent, bool useAVFrame)
    : QObject(parent)
    , q_ptr(parent)
    , avframe {nullptr, AVFrameDeleter}
{
    qDebug() << QString("(0x%1) ").arg((size_t)(this), 0, 16)
             << QString("Direct renderer private created");

    using namespace std::placeholders;
    if (useAVFrame) {
        av_target.push = std::bind(&Video::DirectRendererPrivate::pushAVFrameBuffer, this, _1);
        return;
    }
    sinkTarget_.pullFrame = std::bind(&Video::DirectRendererPrivate::pullFrameBuffer, this, _1);
    sinkTarget_.pushFrame = std::bind(&Video::DirectRendererPrivate::pushFrameBuffer, this, _1);
}

/// Constructor
Video::DirectRenderer::DirectRenderer(const QString& id, const QSize& res, bool useAVFrame)
    : Renderer(id, res)
    , d_ptr(std::make_unique<DirectRendererPrivate>(this, useAVFrame))
{
    setObjectName("Video::DirectRenderer:" + id);
    qDebug() << QString("(0x%1) ").arg((size_t)(this), 0, 16) << QString("Direct renderer created");
}

/// Destructor
Video::DirectRenderer::~DirectRenderer()
{
    QMutexLocker lk(mutex());
    stopRendering();

    d_ptr.reset();

    qDebug() << QString("(0x%1) ").arg((size_t)(this), 0, 16)
             << QString("Direct renderer destroyed");
}

void
Video::DirectRenderer::startRendering()
{
    qDebug() << QString("(0x%1) ").arg((size_t)(this), 0, 16)
             << QString("Starting direct renderer");

    Video::Renderer::d_ptr->m_isRendering = true;
    emit started();
}
void
Video::DirectRenderer::stopRendering()
{
    qDebug() << QString("(0x%1) ").arg((size_t)(this), 0, 16)
             << QString("Stopping direct renderer");

    Video::Renderer::d_ptr->m_isRendering = false;
    emit stopped();
}
void
Video::DirectRenderer::configureTarget(bool useAVFrame)
{
    d_ptr->configureTarget(useAVFrame);
}

DRing::SinkTarget::FrameBufferPtr
Video::DirectRendererPrivate::pullFrameBuffer(std::size_t bytes)
{
    QMutexLocker lk(q_ptr->mutex());
    // TODO. Use a memory pool to reuse these frames.
    return std::move(std::make_unique<DRing::FrameBuffer>(bytes));
}

void
Video::DirectRendererPrivate::configureTarget(bool useAVFrame)
{
    using namespace std::placeholders;
    if (useAVFrame) {
        sinkTarget_.pullFrame = nullptr;
        sinkTarget_.pushFrame = nullptr;
        av_target.push = std::bind(&Video::DirectRendererPrivate::pushAVFrameBuffer, this, _1);
        return;
    }
    sinkTarget_.pullFrame = std::bind(&Video::DirectRendererPrivate::pullFrameBuffer, this, _1);
    sinkTarget_.pushFrame = std::bind(&Video::DirectRendererPrivate::pushFrameBuffer, this, _1);
    av_target.push = nullptr;
}

void
Video::DirectRendererPrivate::pushFrameBuffer(DRing::SinkTarget::FrameBufferPtr buf)
{
    if (not q_ptr->isRendering())
        return;

    {
        QMutexLocker lk(q_ptr->mutex());
        daemonFramePtr_ = std::move(buf);
    }

    emit q_ptr->frameUpdated();
}

void
Video::DirectRendererPrivate::pushAVFrameBuffer(std::unique_ptr<DRing::VideoFrame> frame)
{
    if (not q_ptr->isRendering())
        return;
    {
        QMutexLocker lk(q_ptr->mutex());
        avframe = std::move(frame->getFrame());
    }
    emit q_ptr->frameUpdated();
}

std::unique_ptr<AVFrame, void (*)(AVFrame*)>
Video::DirectRenderer::currentAVFrame() const
{
    if (not isRendering())
        return {nullptr, AVFrameDeleter};
    QMutexLocker lock(mutex());
    return std::move(d_ptr->avframe);
}

std::unique_ptr<lrc::api::video::FrameBufferBase>
Video::DirectRenderer::currentFrame() const
{
    if (d_ptr->frameCounter_ % 100 == 0) {
        qDebug() << QString("(0x%1) ").arg((size_t)(this), 0, 16)
                 << QString("currentFrame %1").arg(d_ptr->frameCounter_);
    }
    d_ptr->frameCounter_++;

    if (not isRendering())
        return {};

    QMutexLocker lock(mutex());

    if (not d_ptr->daemonFramePtr_)
        return {};
    return std::move(d_ptr->daemonFramePtr_);
}

const DRing::SinkTarget&
Video::DirectRenderer::sinkTarget() const
{
    return d_ptr->sinkTarget_;
}

const DRing::AVSinkTarget&
Video::DirectRenderer::avSinkTarget() const
{
    return d_ptr->av_target;
}

Video::Renderer::ColorSpace
Video::DirectRenderer::colorSpace() const
{
#ifdef Q_OS_DARWIN
    return Video::Renderer::ColorSpace::RGBA;
#else
    return Video::Renderer::ColorSpace::BGRA;
#endif
}

#include <directrenderer.moc>

#endif
