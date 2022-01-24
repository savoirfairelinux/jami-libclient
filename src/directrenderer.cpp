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
#include "video_frame_buffer.h"

Video::DirectRenderer::DirectRenderer(const QString& id, const QSize& res)
    : Renderer(id, res)
{
    setObjectName("Video::DirectRenderer:" + id);
    qDebug() << QString("(0x%1) ").arg((size_t)(this), 0, 16) << QString("Direct renderer created");
    using namespace std::placeholders;
    sinkTarget_.pullFrame = nullptr;
    sinkTarget_.pushFrame = std::bind(&Video::DirectRenderer::pushFrameBuffer, this, _1);
    sinkTarget_.bufferType = VideoBufferType::AV_FRAME;
}

Video::DirectRenderer::~DirectRenderer()
{
    QMutexLocker lk(mutex());
    stopRendering();

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

VideoFrameBufferIfPtr Video::DirectRenderer::pullFrameBuffer(std::size_t /*bytes*/)
{
    qCritical() << QString("(0x%1) ").arg((size_t)(this), 0, 16)
                << QString("Unexpected call to an unimplemented method");
    return {};
}

void
Video::DirectRenderer::configureTarget()
{
    using namespace std::placeholders;

    sinkTarget_.pullFrame = nullptr;
    sinkTarget_.pushFrame = std::bind(&Video::DirectRenderer::pushFrameBuffer, this, _1);
    sinkTarget_.bufferType = VideoBufferType::AV_FRAME;
}

void
Video::DirectRenderer::pushFrameBuffer(VideoFrameBufferIfPtr buf)
{
    if (not isRendering())
        return;

    {
        QMutexLocker lk(mutex());
        daemonFramePtr_ = std::move(buf);
    }

    emit frameUpdated();
}

std::unique_ptr<AVFrame, void (*)(AVFrame*)>
Video::DirectRenderer::currentAVFrame() const
{
    if (isRendering())
        return {nullptr, nullptr};
    QMutexLocker lock(mutex());
    // We only give a copy of the raw, we still own the buffer.
    // TODO. Should we give the ownership
    std::unique_ptr<AVFrame, void (*)(AVFrame*)> avframe = {daemonFramePtr_->avframe(), nullptr};
    return avframe;
}

VideoFrameBufferIfPtr
Video::DirectRenderer::currentFrame() const
{
    if (frameCounter_ % 100 == 0) {
        qDebug() << QString("(0x%1) ").arg((size_t)(this), 0, 16)
                 << QString("currentFrame %1").arg(frameCounter_);
    }
    frameCounter_++;

    if (not isRendering())
        return {};

    QMutexLocker lock(mutex());

    if (not daemonFramePtr_)
        return {};
    return std::move(daemonFramePtr_);
}

const DRing::SinkTarget&
Video::DirectRenderer::sinkTarget() const
{
    return sinkTarget_;
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
