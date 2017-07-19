/****************************************************************************
 *   Copyright (C) 2012-2017 Savoir-faire Linux                          *
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

#include "private/videorenderermanager.h"
#include "video/resolution.h"
#include "private/videorenderer_p.h"

#include "videomanager_interface.h"

namespace Video {

class DirectRendererPrivate : public QObject
{
    Q_OBJECT
public:
    DirectRendererPrivate(Video::DirectRenderer* parent);
    DRing::SinkTarget::FrameBufferPtr requestFrameBuffer(std::size_t bytes);
    void onNewFrame(DRing::SinkTarget::FrameBufferPtr buf);

    DRing::SinkTarget target;
    mutable QMutex directmutex;
    mutable DRing::SinkTarget::FrameBufferPtr daemonFramePtr_;
private:
    Video::DirectRenderer* q_ptr;
};

} // namespace Video

Video::DirectRendererPrivate::DirectRendererPrivate(Video::DirectRenderer* parent) :
QObject(parent),
q_ptr(parent)
{
    using namespace std::placeholders;
    target.pull = std::bind(&Video::DirectRendererPrivate::requestFrameBuffer, this, _1);
    target.push = std::bind(&Video::DirectRendererPrivate::onNewFrame, this, _1);
}

///Constructor
Video::DirectRenderer::DirectRenderer(const QByteArray& id, const QSize& res) :
Renderer(id, res),
d_ptr(new DirectRendererPrivate(this))
{
    setObjectName("Video::DirectRenderer:"+id);
}

///Destructor
Video::DirectRenderer::~DirectRenderer()
{
}

void Video::DirectRenderer::startRendering()
{
   Video::Renderer::d_ptr->m_isRendering = true;
   emit started();
}
void Video::DirectRenderer::stopRendering ()
{
   Video::Renderer::d_ptr->m_isRendering = false;
   emit stopped();
}

DRing::SinkTarget::FrameBufferPtr Video::DirectRendererPrivate::requestFrameBuffer(std::size_t bytes)
{
    QMutexLocker lk(q_ptr->mutex());
    if (not daemonFramePtr_)
        daemonFramePtr_.reset(new DRing::FrameBuffer);
    daemonFramePtr_->storage.resize(bytes);
    daemonFramePtr_->ptr = daemonFramePtr_->storage.data();
    daemonFramePtr_->ptrSize = bytes;
    return std::move(daemonFramePtr_);
}

void Video::DirectRendererPrivate::onNewFrame(DRing::SinkTarget::FrameBufferPtr buf)
{
    if (not q_ptr->isRendering())
        return;

    {
        QMutexLocker lk(q_ptr->mutex());
        daemonFramePtr_ = std::move(buf);
    }

    emit q_ptr->frameUpdated();
}

Video::Frame Video::DirectRenderer::currentFrame() const
{
    if (not isRendering())
        return {};

    QMutexLocker lock(mutex());
    if (not d_ptr->daemonFramePtr_)
        return {};

    Video::Frame frame;
    frame.storage = std::move(d_ptr->daemonFramePtr_->storage);
    frame.ptr = frame.storage.data();
    frame.size = frame.storage.size();
    return std::move(frame);
}

const DRing::SinkTarget& Video::DirectRenderer::target() const
{
    return d_ptr->target;
}

Video::Renderer::ColorSpace Video::DirectRenderer::colorSpace() const
{
#ifdef Q_OS_DARWIN
   return Video::Renderer::ColorSpace::RGBA;
#else
   return Video::Renderer::ColorSpace::BGRA;
#endif
}

#include <directrenderer.moc>
