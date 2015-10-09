/****************************************************************************
 *   Copyright (C) 2012-2015 by Savoir-Faire Linux                          *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com>      *
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

namespace Video {

class DirectRendererPrivate : public QObject
{
   Q_OBJECT
public:
   DirectRendererPrivate(Video::DirectRenderer* parent);
   DRing::SinkTarget::FrameBufferPtr requestFrameBuffer();
   void onNewFrame(DRing::SinkTarget::FrameBufferPtr buf);

   void swapFrames() const;
   mutable QMutex directmutex;
   DRing::SinkTarget target;
   mutable DRing::SinkTarget::FrameBufferPtr jojo_;
   mutable DRing::SinkTarget::FrameBufferPtr bernard_;

private:
    Video::DirectRenderer* q_ptr;
};

}

Video::DirectRendererPrivate::DirectRendererPrivate(Video::DirectRenderer* parent) :
QObject(parent),
q_ptr(parent),
jojo_(new DRing::FrameBuffer),
bernard_(new DRing::FrameBuffer)
{
    using namespace std::placeholders;
    target.pull = std::bind(&Video::DirectRendererPrivate::requestFrameBuffer, this);
    target.push = std::bind(&Video::DirectRendererPrivate::onNewFrame, this, _1);
}

///Constructor
Video::DirectRenderer::DirectRenderer(const QByteArray& id, const QSize& res):
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

DRing::SinkTarget::FrameBufferPtr Video::DirectRendererPrivate::requestFrameBuffer()
{
    QMutexLocker lock(&directmutex);
    return std::move(jojo_);
}

void Video::DirectRendererPrivate::onNewFrame(DRing::SinkTarget::FrameBufferPtr buf)
{
    if (!q_ptr->isRendering()) {
        return;
    }
    {
        QMutexLocker lock(&directmutex);
        jojo_ = std::move(buf);
    }

    emit q_ptr->frameUpdated();
}

// Called only by currentFrame
void Video::DirectRendererPrivate::swapFrames() const
{
    QMutexLocker lock(&directmutex);
    jojo_.swap(bernard_);
}

const QByteArray& Video::DirectRenderer::currentFrame() const
{
    d_ptr->swapFrames();
    Video::Renderer::d_ptr->m_pFrame = reinterpret_cast<char*>(d_ptr->bernard_->data.data());
    Video::Renderer::d_ptr->m_FrameSize = d_ptr->bernard_->data.size();
    if (Video::Renderer::d_ptr->m_pFrame && Video::Renderer::d_ptr->m_FrameSize) {
        Video::Renderer::d_ptr->m_Content.setRawData(Video::Renderer::d_ptr->m_pFrame,
                                    Video::Renderer::d_ptr->m_FrameSize);
    }
    return Video::Renderer::d_ptr->m_Content;
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
